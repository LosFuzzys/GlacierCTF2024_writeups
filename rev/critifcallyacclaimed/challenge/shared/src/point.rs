use std::{num::TryFromIntError, ops};

#[cfg(feature = "serialize")]
use serde::Serialize;
#[cfg(feature = "deserialize")]
use serde::Deserialize;

#[cfg_attr(feature = "debug", derive(Debug))]
#[cfg_attr(feature = "serialize", derive(Serialize))]
#[cfg_attr(feature = "deserialize", derive(Deserialize))]
#[derive(Copy, Clone, PartialEq, Eq)]
pub struct Point {
    pub x: i32,
    pub y: i32
}

impl From<(i32, i32)> for Point {
    fn from(value: (i32, i32)) -> Self {
        Point { x: value.0, y: value.1 }
    }
}

impl Into<(i32, i32)> for Point {
    fn into(self) -> (i32, i32) {
        (self.x, self.y)
    }
}

impl TryFrom<(usize, usize)> for Point {
    type Error = TryFromIntError;
    fn try_from(value: (usize, usize)) -> Result<Self, Self::Error> {
        Ok(Point { x: value.0.try_into()?, y: value.1.try_into()? })
    }
}

impl Point {
    #[inline]
    pub fn new(x: i32, y: i32) -> Point {
        Point {
            x,
            y,
        }
    }

    #[inline]
    pub fn dist(&self, pos: Point) -> Option<f32> {
        Some((((self.x - pos.x).checked_pow(2)? + (self.y - pos.y).checked_pow(2)?) as f32).sqrt())
    }

    #[inline]
    pub fn signum(&self) -> Point {
        Point {
            x: self.x.signum(),
            y: self.y.signum()
        }
    }

    #[inline]
    pub fn abs(&self) -> Point {
        Point {
            x: self.x.abs(),
            y: self.y.abs()
        }
    }

    #[inline]
    pub fn sum(&self) -> i32 {
        self.x + self.y
    }
}

impl ops::Add<Point> for Point {
    type Output = Point;
    fn add(self, rhs: Point) -> Point {
        Point {
            x: self.x + rhs.x,
            y: self.y + rhs.y
        }
    }
}

impl ops::Sub<Point> for Point {
    type Output = Point;
    fn sub(self, rhs: Point) -> Point {
        Point {
            x: self.x - rhs.x,
            y: self.y - rhs.y
        }
    }
}

impl ops::Mul<Point> for Point {
    type Output = Point;
    fn mul(self, rhs: Point) -> Point {
        Point {
            x: self.x * rhs.x,
            y: self.y * rhs.y
        }
    }
}
