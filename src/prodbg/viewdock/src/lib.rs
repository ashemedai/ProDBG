mod error;
pub use self::error::Error;
extern crate xml;
use std::io::prelude::*;
use std::fs::File;
use std::io::BufReader;
//use std::io;

use xml::reader::{EventReader};
use xml::writer::{EventWriter, EmitterConfig, XmlEvent};
use xml::writer::Result as XmlResult;
use xml::reader::Result as XmlReaderResult;
use xml::reader::XmlEvent as XmlReaderEvent;
use xml::attribute::OwnedAttribute;

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

#[derive(Debug, PartialEq, Clone, Copy)]
pub struct SplitHandle(pub u64);

#[derive(Debug, Default, Clone, Copy)]
pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub name: String,
    pub rect: Rect
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Direction {
    Vertical,
    Horizontal,
    Full,
}

#[derive(Debug, Clone)]
pub struct Container {
    pub docks: Vec<Dock>,
    pub rect: Rect,
}

#[derive(Debug)]
pub struct Split {
    /// left/top slipit
    pub left: Option<SplitHandle>,
    /// right/bottom split
    pub right: Option<SplitHandle>,
    /// left/top docks
    pub left_docks: Container,
    /// right/top docks
    pub right_docks: Container,
    /// ratioage value of how much of each side that is visible. 1.0 = right/bottom fully visible
    pub ratio: f32,
    /// Direction of the split
    pub direction: Direction,
    /// Handle of the spliter
    pub handle: SplitHandle,
    /// Rect
    rect: Rect,
}

#[derive(Debug)]
pub struct Workspace {
    pub splits: Vec<Split>,
    pub rect: Rect,
    /// border size of the windows (in pixels)
    pub window_border: f32,
    handle_counter: SplitHandle,
}

impl Rect {
    pub fn new(x: f32, y: f32, width: f32, height: f32) -> Rect {
        Rect {
            x: x,
            y: y,
            width: width,
            height: height
        }
    }

    pub fn save<W: Write>(&self, writer: &mut EventWriter<W>) -> XmlResult<()> {
        try!(writer.write(XmlEvent::start_element("rect")
                          .attr("x", &format!("{}", self.x))
                          .attr("y", &format!("{}", self.y))
                          .attr("width", &format!("{}", self.width))
                          .attr("height", &format!("{}", self.height))));
        writer.write(XmlEvent::end_element())
    }

    // TODO(collin): Return error here

    pub fn load(attributes: &Vec<OwnedAttribute>) -> Rect {
        let mut rect = Rect::new(0.0, 0.0, 1024.0, 768.0);

        for attrib in attributes {
            match attrib.name.local_name.as_ref() {
                "x" => { rect.x = attrib.value.parse::<f32>().unwrap(); },
                "y" => { rect.y = attrib.value.parse::<f32>().unwrap(); },
                "width" => { rect.width = attrib.value.parse::<f32>().unwrap(); },
                "height" => { rect.height = attrib.value.parse::<f32>().unwrap(); },
                _ => (),
            }
        }

        rect
    }
}

impl Rect {
    fn is_inside(v: (f32, f32), rect: Rect) -> bool {
        let x0 = rect.x;
        let y0 = rect.y;
        let x1 = x0 + rect.width;
        let y1 = y0 + rect.height;

        //println!("checking {:?} inside {:?}", v, rect);

        if (v.0 >= x0 && v.0 < x1) && (v.1 >= y0 && v.1 < y1) {
            true
        } else {
            false
        }
    }
}

impl Dock {
    fn new(dock_handle: DockHandle) -> Dock {
        Dock {
            handle: dock_handle,
            name: "".to_owned(),
            rect: Rect::default(),
        }
    }
}

impl Container {
    pub fn new() -> Container {
        Container {
            docks: Vec::new(),
            rect: Rect::default(),
        }
    }

    pub fn set_dock_name(&mut self, name: &String, handle: DockHandle) -> bool {
        for h in &mut self.docks {
            if h.handle == handle {
                h.name = name.clone();
                return true;
            }
        }

        false
    }

    pub fn find_handle(&self, handle: DockHandle) -> bool {
        for dock in &self.docks {
            if dock.handle == handle {
                return true;
            }
        }

        false
    }

    pub fn remove_handle(&mut self, handle: DockHandle) -> bool {
        for i in (0..self.docks.len()).rev() {
            if self.docks[i].handle == handle {
                //println!("Removed dock handle {:?}", handle);
                self.docks.swap_remove(i);
                return true;
            }
        }

        false
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        for dock in &self.docks {
            if dock.handle == handle {
                return Some(self.rect);
            }
        }

        None
    }

    pub fn is_inside(&self, pos: (f32, f32)) -> Option<(DockHandle, f32)> {
        for dock in &self.docks {
            if Rect::is_inside(pos, self.rect) {
                return Some((dock.handle, dock.rect.width * dock.rect.height));
            }
        }

        None
    }

    pub fn save<W: Write>(&self, name: &str, writer: &mut EventWriter<W>) -> XmlResult<()> {
        if self.docks.len() == 0 {
            return Ok(());
        }

        try!(writer.write(XmlEvent::start_element(name)));

        for dock in &self.docks {
            try!(writer.write(XmlEvent::start_element("dock")
                              .attr("handle", &format!("{}", dock.handle.0))
                              .attr("name", &format!("{}", dock.name))));
            try!(writer.write(XmlEvent::end_element()));
        }

        writer.write(XmlEvent::end_element())
    }

    pub fn load<E: Read>(parser: &mut EventReader<E>, side_name: &str) -> Container {
        let mut container = Container::new();

        //println!("Loding docks...");

        loop {
            match parser.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    if name.local_name == "dock" {
                        let mut dock = Dock::new(DockHandle(0));

                        for attr in attributes {
                            match attr.name.local_name.as_ref() {
                                "handle" => dock.handle = DockHandle(attr.value.parse::<u64>().unwrap()),
                                "name" => dock.name = attr.value.to_owned(),
                                _ => (),
                            }
                        }

                        //println!("found dock!");

                        container.docks.push(dock);
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == side_name {
                        break;
                    }
                },
                Err(_) => break,

                _ => (),
            }
        }

        container
    }

}

impl Split {
    pub fn new(direction: Direction, ratio: f32, handle: SplitHandle) -> Split {
        Split {
            left: None,
            right: None,
            left_docks: Container::new(),
            right_docks: Container::new(),
            ratio: ratio,
            direction: direction,
            handle: handle,
            rect: Rect::default(),
        }
    }

    pub fn is_left_zero_and_none(&self) -> bool {
        self.left_docks.docks.len() == 0 && self.left.is_none()
    }

    pub fn is_right_zero_and_none(&self) -> bool {
        self.right_docks.docks.len() == 0 && self.right.is_none()
    }

    fn adjust_ratio_after_add(&mut self) {
        if self.right_docks.docks.len() == 0 {
            self.ratio = 1.0;
        } else if self.left_docks.docks.len() == 0 {
            self.ratio = 0.0;
        } else {
            self.ratio = 0.5;
        }
    }

    pub fn no_split(&mut self, direction: Direction, dock_handle: DockHandle) -> bool {
        // hack!
        if self.direction == Direction::Full {
            self.direction = direction;
        }

        if self.is_left_zero_and_none() {
            //println!("no_split: is_left_zero_and_none");
            self.left_docks.docks.push(Dock::new(dock_handle));
            self.adjust_ratio_after_add();
            return true;
        }

        if self.is_right_zero_and_none() {
            //println!("no_split: is_right_zero_and_none");
            self.right_docks.docks.push(Dock::new(dock_handle));
            self.adjust_ratio_after_add();
            return true;
        }

        false
    }

    pub fn split_left(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) -> Option<Split> {
        if Self::no_split(self, direction, dock_handle) {
            return None;
        }

        let mut split = Split::new(direction, 0.5, split_handle);
        split.right_docks = self.left_docks.clone();
        split.left_docks.docks.push(Dock::new(dock_handle));
        self.ratio = 0.5;
        self.left = Some(split_handle);
        self.left_docks.docks.clear();

        Some(split)
    }

    pub fn split_right(&mut self, split_handle: SplitHandle, dock_handle: DockHandle, direction: Direction) -> Option<Split> {
        if Self::no_split(self, direction, dock_handle) {
            return None;
        }

        let mut split = Split::new(direction, 0.5, split_handle);
        split.left_docks = self.right_docks.clone();
        split.right_docks.docks.push(Dock::new(dock_handle));
        self.ratio = 0.5;
        self.right = Some(split_handle);
        self.right_docks.docks.clear();

        Some(split)
    }


    pub fn get_sizer_from_rect_horizontal(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x, rect.y + rect.height, rect.width, size)
    }

    pub fn get_sizer_from_rect_vertical(rect: Rect, size: f32) -> Rect {
        Rect::new(rect.x + rect.width, rect.y, size, rect.height)
    }

    pub fn is_hovering_rect(&self, pos: (f32, f32), border_size: f32, rect: Rect) -> bool {
        match self.direction {
            Direction::Horizontal => Rect::is_inside(pos, Self::get_sizer_from_rect_horizontal(rect, border_size)),
            Direction::Vertical => Rect::is_inside(pos, Self::get_sizer_from_rect_vertical(rect, border_size)),
            Direction::Full => false,
        }
    }

    pub fn map_rect_to_delta(&self, delta: (f32, f32)) -> f32 {
        match self.direction {
            Direction::Vertical => -delta.0 / self.rect.width,
            Direction::Horizontal => -delta.1 / self.rect.height,
            _ => 0.0,
        }
    }

    pub fn change_ratio(&mut self, delta: (f32, f32)) {
        let scale = Self::map_rect_to_delta(self, delta);

        self.ratio += scale;

        if self.ratio < 0.01 {
            self.ratio = 0.01;
        }

        if self.ratio > 0.99 {
            self.ratio = 0.99;
        }
    }

    fn pad(level: i32) {
        for _ in 0..level {
            print!(" ");
        }
    }

    pub fn dump_info(&self, level: i32) {
        let l_count = self.left_docks.docks.len();
        let r_count = self.right_docks.docks.len();

        Self::pad(level);

        println!("Split - Dir {:?} - handle {} - ratio {} count ({}, {}) - left ({:?}) right ({:?}) - rect {} {} - {} {}",
        self.direction, self.handle.0, self.ratio, l_count, r_count,
        self.left, self.right, self.rect.x, self.rect.y, self.rect.width, self.rect.height);

        for d in &self.left_docks.docks {
            Self::pad(level);
            println!("left dock handle - {}", d.handle.0);
        }

        for d in &self.right_docks.docks {
            Self::pad(level);
            println!("right dock handle - {}", d.handle.0);
        }
    }

    fn write_child<W: Write>(child: Option<SplitHandle>, writer: &mut EventWriter<W>, name: &str) -> XmlResult<()> {
        if child == None {
            try!(writer.write(XmlEvent::start_element(name).attr("con", "None")));
        } else {
            try!(writer.write(XmlEvent::start_element(name).attr("con", &format!("{}", child.unwrap().0))));
        }

        writer.write(XmlEvent::end_element())
    }

    pub fn save<W: Write>(&self, writer: &mut EventWriter<W>) -> XmlResult<()> {
        try!(writer.write(XmlEvent::start_element("split")));

        try!(writer.write(XmlEvent::start_element("ratio").attr("v", &format!("{}", self.ratio))));
        try!(writer.write(XmlEvent::end_element()));
        try!(writer.write(XmlEvent::start_element("direction").attr("v", &format!("{:?}", self.direction))));
        try!(writer.write(XmlEvent::end_element()));
        try!(writer.write(XmlEvent::start_element("handle").attr("v", &format!("{}", self.handle.0))));
        try!(writer.write(XmlEvent::end_element()));

        try!(self.left_docks.save("left_docks", writer));
        try!(self.right_docks.save("right_docks", writer));

        try!(Self::write_child(self.left, writer, "left"));
        try!(Self::write_child(self.right, writer, "right"));

        writer.write(XmlEvent::end_element())
    }

    pub fn parse_child(attributes: &Vec<OwnedAttribute>) -> Option<SplitHandle> {
        for attr in attributes {
            if attr.name.local_name == "con" {
                if attr.value == "None" {
                    return None;
                }

                return Some(SplitHandle(attr.value.parse::<u64>().unwrap()));
            }
        }

        None
    }

    pub fn load<E: Read>(reader: &mut EventReader<E>) -> Split {
        let mut split = Split::new(Direction::Full, 0.5, SplitHandle(0));

        //println!("loading split...");

        loop {
            match reader.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    //println!("Name {}", name);
                    match name.local_name.as_ref() {
                        "left_docks" => split.left_docks = Container::load(reader, "left_docks"),
                        "right_docks" => split.right_docks = Container::load(reader, "right_docks"),
                        "left" => split.left = Self::parse_child(&attributes),
                        "right" => split.right = Self::parse_child(&attributes),
                        "ratio" => split.ratio = attributes[0].value.parse::<f32>().unwrap(),
                        "handle" => split.handle = SplitHandle(attributes[0].value.parse::<u64>().unwrap()),
                        "direction" => {
                            split.direction = match attributes[0].value.as_ref() {
                                "Vertical" => Direction::Vertical,
                                "Horizontal" => Direction::Horizontal,
                                _ => Direction::Full,
                            };
                        }
                        _ => (),
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == "split" {
                        break;
                    }
                },
                Err(_) => break,

                _ => (),
            }
        }

        split
    }
}

impl Workspace {
    /// Construct a new workspace. The rect has to be y >= 0, x >= 0, width > 0 and height > 0
    pub fn new(rect: Rect) -> std::io::Result<Workspace> {
        if rect.x < 0.0 || rect.y < 0.0 || rect.width <= 0.0 || rect.height <= 0.0 {
            //return Err(Error::IllegalSize(write!("Illegal rect {} {} - {} {}",
            //                              rect.x, rect.y, rect.width, rect.height)));
            //return Err(Error::IllegalSize("Illegal rect size".to_owned()));
        }

        Ok(Workspace {
            splits: vec![Split::new(Direction::Full, 0.0, SplitHandle(0))],
            rect: rect,
            window_border: 4.0,
            handle_counter: SplitHandle(0),
        })
    }

    #[inline]
    fn next_handle(&mut self) -> SplitHandle {
        self.handle_counter.0 += 1;
        self.handle_counter
    }

    pub fn calc_horizontal_sizing(rect: Rect, ratio: f32) -> (Rect, Rect) {
        let h = rect.height * ratio;

        let rect_top = Rect::new(rect.x, rect.y, rect.width, h);
        let rect_bottom = Rect::new(rect.x, rect.y + h, rect.width, rect.height - h);

        (rect_top, rect_bottom)
    }

    pub fn calc_vertical_sizing(rect: Rect, ratio: f32) -> (Rect, Rect) {
        let w = rect.width * ratio;

        let rect_left = Rect::new(rect.x, rect.y, w, rect.height);
        let rect_right = Rect::new(rect.x + w, rect.y, rect.width - w, rect.height);

        (rect_left, rect_right)
    }

    fn calc_rects(direction: Direction, rect: Rect, ratio: f32) -> (Rect, Rect) {
        match direction {
            Direction::Vertical => Self::calc_vertical_sizing(rect, ratio),
            Direction::Horizontal => Self::calc_horizontal_sizing(rect, ratio),
            Direction::Full => (rect, rect),
        }
    }

    pub fn split(&mut self, dock_handle: DockHandle, direction: Direction) {
        // If we have only one split we right now split on the left side
        if self.splits.len() == 1 {
            let handle = self.next_handle();
            self.splits[0].split_left(handle, dock_handle, direction);
        }
    }

    fn find_split_by_handle(splits: &Vec<Split>, handle: SplitHandle) -> usize {
        for i in 0..splits.len() {
            if splits[i].handle == handle {
                return i;
            }
        }

        panic!("Should never happen!")
    }

    fn recursive_update(splits: &mut Vec<Split>, rect: Rect, handle: SplitHandle) {
        let i = Self::find_split_by_handle(splits, handle);

        let rects = Self::calc_rects(splits[i].direction, rect, splits[i].ratio);

        if let Some(split_handle) = splits[i].left {
            Self::recursive_update(splits, rects.0, split_handle);
        }

        if let Some(split_handle) = splits[i].right {
            Self::recursive_update(splits, rects.1, split_handle);
        }

        splits[i].rect = rect;
        splits[i].left_docks.rect = rects.0;
        splits[i].right_docks.rect = rects.1;
    }

    pub fn split_by_dock_handle(&mut self, direction: Direction, find_handle: DockHandle, handle: DockHandle) {
        let split_handle = self.next_handle();
        let mut temp_splits = Vec::new();

        //println!("split_by_dock_handle");

        for split in &mut self.splits {
            if split.left_docks.find_handle(find_handle) {
                if let Some(s) = split.split_left(split_handle, handle, direction) {
                    temp_splits.push(s);
                }
            }

            if split.right_docks.find_handle(find_handle) {
                if let Some(s) = split.split_right(split_handle, handle, direction) {
                    temp_splits.push(s);
                }
            }
        }

        for split in temp_splits {
            self.splits.push(split);
        }
    }

    fn delete_handle(splits: &mut Vec<Split>, handle: DockHandle) {
        for i in 0..splits.len() {
            if splits[i].left_docks.remove_handle(handle) {
                return;
            }

            if splits[i].right_docks.remove_handle(handle) {
                return;
            }
        }
    }

    pub fn new_split(&mut self, handle: DockHandle, direction: Direction) {
        if self.splits.len() == 1 {
            self.splits[0].split_left(SplitHandle(0), handle, direction);
        }
    }

    pub fn get_rect_by_handle(&self, handle: DockHandle) -> Option<Rect> {
        for split in &self.splits {
            if let Some(rect) = split.left_docks.get_rect_by_handle(handle) {
                return Some(rect);
            }

            if let Some(rect) = split.right_docks.get_rect_by_handle(handle) {
                return Some(rect);
            }
        }

        None
    }

    pub fn get_hover_dock(&self, pos: (f32, f32)) -> Option<DockHandle> {
        let mut dock_handle = None;
        let mut last_size = 100000000.0;

        for split in &self.splits {
            if let Some(data) = split.left_docks.is_inside(pos) {
                if data.1 < last_size {
                    dock_handle = Some(data.0);
                    last_size = data.1;
                }
            }

            if let Some(data) = split.right_docks.is_inside(pos) {
                if data.1 < last_size {
                    dock_handle = Some(data.0);
                    last_size = data.1;
                }
            }

        }

        dock_handle
    }


    fn clean_splits(splits: &mut Vec<Split>) -> bool {
        if splits.len() == 1 {
            return false;
        }

        for i in (0..splits.len()).rev() {
            if splits[i].is_left_zero_and_none() && splits[i].is_right_zero_and_none() {
                let handle = Some(splits[i].handle);

                for split in splits.iter_mut() {
                    if split.left == handle {
                        split.ratio = 0.0;
                        split.left = None;
                    }

                    if split.right == handle {
                        split.ratio = 1.0;
                        split.right = None;
                    }
                }

                splits.swap_remove(i);
                return true;
            }
        }

        false
    }

    fn cleanup_after_delete(splits: &mut Vec<Split>) {
        loop {
            if !Self::clean_splits(splits) {
                break;
            }
        }

        // If there is nothing left (empty space) we switch the direction
        // to full for the first splitter

        if splits.len() == 1 {
            if splits[0].is_left_zero_and_none() && splits[0].is_right_zero_and_none() {
                splits[0].direction = Direction::Full;
            }
        }
    }

    fn adjust_percentages(splits: &mut Vec<Split>) {
        for split in splits {
            if split.is_left_zero_and_none() {
                split.ratio = 0.0;
            } else if split.is_right_zero_and_none() {
                split.ratio = 1.0;
            }
        }
    }

    pub fn update(&mut self) {
        Self::recursive_update(&mut self.splits, self.rect, SplitHandle(0));
    }

    pub fn drag_sizer(&mut self, handle: SplitHandle, delta: (f32, f32)) {
        for split in &mut self.splits {
            if split.handle == handle {
                return split.change_ratio(delta);
            }
        }
    }

    pub fn is_hovering_sizer(&self, pos: (f32, f32)) -> Option<(SplitHandle, Direction)> {
        for split in &self.splits {
            if split.is_hovering_rect(pos, 8.0, split.left_docks.rect) {
                return Some((split.handle, split.direction));
            }
        }

        None
    }

    fn recursive_dump(splits: &Vec<Split>, handle: SplitHandle, level: i32) {
        let i = Self::find_split_by_handle(splits, handle);

        splits[i].dump_info(level);

        if let Some(split_handle) = splits[i].left {
            Self::recursive_dump(splits, split_handle, level + 1);
        }

        if let Some(split_handle) = splits[i].right {
            Self::recursive_dump(splits, split_handle, level + 1);
        }
    }

    // testing

    pub fn dump_tree(&self) {
        Self::recursive_dump(&self.splits, SplitHandle(0), 0);
    }

    pub fn dump_tree_linear(&self) {
        for split in &self.splits {
            split.dump_info(0);
        }
    }

    pub fn delete_by_handle(&mut self, handle: DockHandle) {
        Self::delete_handle(&mut self.splits, handle);
        Self::cleanup_after_delete(&mut self.splits);
        Self::adjust_percentages(&mut self.splits);
    }

    pub fn set_name_to_handle(&mut self, name: &String, handle: DockHandle) {
        for split in &mut self.splits {
            if split.left_docks.set_dock_name(name, handle) { break; }
            if split.right_docks.set_dock_name(name, handle) { break; }
        }
    }

    pub fn save(&self, file_name: &str) -> XmlResult<()> {
        let mut file = try!(File::create(file_name));
        let mut w = EmitterConfig::new().perform_indent(true).create_writer(&mut file);

        try!(w.write(XmlEvent::start_element("workspace")));

        try!(self.rect.save(&mut w));

        //let mut t = String::new();
        //let t = format!("{}", self.rect.x);

        for split in &self.splits {
            try!(split.save(&mut w));
        }

        try!(w.write(XmlEvent::end_element()));

        println!("saved file");
        Ok(())
    }

    pub fn get_docks(&self) -> Vec<Dock> {
        let mut docks = Vec::new();

        for split in &self.splits {
            for dock in &split.left_docks.docks {
                docks.push(dock.clone());
            }

            for dock in &split.right_docks.docks {
                docks.push(dock.clone());
            }
        }

        docks
    }

    pub fn internal_load(&mut self, filename: &str) -> XmlReaderResult<()> {
        let file = File::open(filename).unwrap();
        let file = BufReader::new(file);

        let mut parser = EventReader::new(file);

        loop {
            match parser.next() {
                Ok(XmlReaderEvent::StartElement { name, attributes, .. }) => {
                    match name.local_name.as_ref() {
                        "rect" => self.rect = Rect::load(&attributes),
                        "split" => {
                            let split = Split::load(&mut parser);
                            //split.dump_info(0);
                            self.splits.push(split);
                        }
                        _ => (),
                    }
                },
                Ok(XmlReaderEvent::EndElement { name }) => {
                    if name.local_name == "workspace" {
                        break;
                    }
                },
                Err(e) => return Err(e),

                _ => (),
            }
        }

        println!("loaded ok!");

        Ok(())
    }

    pub fn load(filename: &str) -> Option<Workspace> {
        let mut ws = Self::new(Rect::new(0.0, 0.0, 1.0, 1.0)).unwrap();

        ws.splits.clear();

        if Self::internal_load(&mut ws, filename).is_ok() {
            //println!("split count {}", ws.splits.len());
            //ws.dump_tree();
            Some(ws)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod test {
    use {Split, Workspace, Rect, DockHandle, Direction};

    fn check_range(inv: f32, value: f32, delta: f32) -> bool {
        (inv - value).abs() < delta
    }

    #[test]
    fn test_validate_x_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(-0.1, 0.0, 1.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_y_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, -0.1, 1.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_width_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 0.0, 1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_height_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 1.0, 0.0)).is_err(), true);
    }

    #[test]
    fn test_validate_width_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, -1.0, 0.0)).is_err(), true);
    }

    #[test]
    fn test_validate_height_less_than_zero() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 0.0, -1.0)).is_err(), true);
    }

    #[test]
    fn test_validate_workspace_ok() {
        assert_eq!(Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).is_ok(), true);
    }

    #[test]
    fn test_split_top() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split_top(DockHandle(1), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.left_docks.docks.len(), 1);
    }

    #[test]
    fn test_split_top_2() {
        let mut ws = Workspace::new(Rect::new(0.0, 0.0, 1024.0, 1024.0)).unwrap();
        ws.split_top(DockHandle(1), Direction::Vertical);
        ws.split_top(DockHandle(2), Direction::Vertical);

        assert_eq!(ws.split.is_some(), true);
        let split = ws.split.unwrap();

        assert_eq!(split.right_docks.docks.len(), 1);
        assert_eq!(split.left_docks.docks.len(), 1);
        assert_eq!(check_range(split.ratio, 0.5, 0.01), true);
    }

    #[test]
    fn test_calc_rect_horz_half() {
        let rects = Split::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.5);

        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 512.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 512.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 512.0, 0.001), true);
    }

    #[test]
    fn test_calc_rect_horz_25_per() {
        let rects = Split::calc_horizontal_sizing(Rect::new(0.0, 0.0, 1024.0, 1024.0), 0.25);

        assert_eq!(check_range(rects.0.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 0.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 0.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 256.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 1024.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
    }

    #[test]
    fn test_calc_rect_horz_25_per_2() {
        let rects = Split::calc_horizontal_sizing(Rect::new(16.0, 32.0, 512.0, 1024.0), 0.25);

        assert_eq!(check_range(rects.0.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.0.y, 32.0, 0.001), true);
        assert_eq!(check_range(rects.0.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.0.height, 256.0, 0.001), true);

        assert_eq!(check_range(rects.1.x, 16.0, 0.001), true);
        assert_eq!(check_range(rects.1.y, 288.0, 0.001), true);
        assert_eq!(check_range(rects.1.width, 512.0, 0.001), true);
        assert_eq!(check_range(rects.1.height, 768.0, 0.001), true);
    }

    #[test]
    fn test_gen_horizontal_size() {
        let border_size = 4.0;
        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect = Split::get_sizer_from_rect_horizontal(rect_in, border_size);

        assert_eq!(check_range(rect.x, rect_in.x, 0.001), true);
        assert_eq!(check_range(rect.y, 60.0, 0.001), true);
        assert_eq!(check_range(rect.width, rect_in.width, 0.001), true);
        assert_eq!(check_range(rect.height, border_size, 0.001), true);
    }

    #[test]
    fn test_gen_vertical_size() {
        let border_size = 4.0;
        let rect_in = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect = Split::get_sizer_from_rect_vertical(rect_in, border_size);

        assert_eq!(check_range(rect.x, 40.0, 0.001), true);
        assert_eq!(check_range(rect.y, rect_in.y, 0.001), true);
        assert_eq!(check_range(rect.width, border_size, 0.001), true);
        assert_eq!(check_range(rect.height, rect_in.height, 0.001), true);
    }

    #[test]
    fn test_inside_horizontal() {
        let border_size = 4.0;
        let rect = Rect::new(10.0, 20.0, 30.0, 40.0);
        let rect_horz = Split::get_sizer_from_rect_horizontal(rect, border_size);

        assert_eq!(Split::is_inside((9.0, 61.0), rect_horz), false);
        assert_eq!(Split::is_inside((11.0, 61.0), rect_horz), true);
    }
}
