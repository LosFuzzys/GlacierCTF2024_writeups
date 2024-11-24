use crate::{player::MAX_PLAYER_HEALTH, point::Point, Direction, ARENA_SIZE, EPSILON, ROUND};

// #[cfg(any(feature = "serialize", feature = "deserialize"))]
#[cfg(feature = "deserialize")]
use serde::Deserialize;
#[cfg(feature = "serialize")]
use serde::Serialize;

pub(crate) const KB_AMOUNT: f32 = ARENA_SIZE as f32 * 0.75;
pub(crate) const FLARE_MODIFIER: f32 = MAX_PLAYER_HEALTH as f32 * 0.7;
pub(crate) const TOWER_DAMAGE: i32 = (MAX_PLAYER_HEALTH as f32 * 0.7) as i32;

type Rad = f32;

#[cfg_attr(feature = "debug", derive(Debug))]
#[cfg_attr(feature = "serialize", derive(Serialize))]
#[cfg_attr(feature = "deserialize", derive(Deserialize))]
#[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(tag = "type"))]
#[derive(Copy, Clone)]
pub enum Mechanic {
    Aoe {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
        size: i32,
    },
    Donut {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
        size: i32,
        inner_size: i32,
    },
    Laser {
        orientation: Rad,
        offsetx: f32,
        offsety: f32,
        thickness: f32,
    },
    Cleave {
        orientation: Rad,
        start: i32,
        end: i32,
    },
    Cone {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
        spread: f32,
        angle: f32,
    },
    Puddle {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
        size: i32,
        duration: u32,
    },
    Tower {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
    },
    Flare {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
    },
    Exaflare {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
        direction: Direction,
    },
    Wind {
        orientation: Rad,
    },
    Knockback {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
    },
    Gaze {
        #[cfg_attr(any(feature = "serialize", feature = "deserialize"), serde(flatten))]
        pos: Point,
    },
    Raidwide {
        damage: u32,
    },
    Pyretic,
    Enrage,
}

impl Mechanic {
    pub fn resolve(self, origin: Point) -> Result<bool, &'static str> {
        Ok(match self {
            Mechanic::Aoe { pos, size } => {
                origin.dist(pos).ok_or("Overflow")? + EPSILON < size as f32
            }
            Mechanic::Donut {
                pos,
                size,
                inner_size,
            } => {
                let dist = origin.dist(pos).ok_or("Overflow")? + EPSILON;
                (inner_size as f32) < dist && dist < size as f32
            }
            Mechanic::Laser {
                orientation,
                offsetx,
                offsety,
                thickness,
            } => {
                (((orientation.cos() * (origin.x as f32 - offsetx)) * ROUND)
                    + (orientation.sin() * (origin.y as f32 - offsety)) * ROUND)
                    .abs()
                    .round()
                    / ROUND
                    < thickness
            }
            Mechanic::Cleave {
                orientation,
                start,
                end,
            } => {
                (((orientation.cos() * (origin.x as f32 - (start + end) as f32 / 2.)) * ROUND)
                    + (orientation.sin() * (origin.y as f32 - (start + end) as f32 / 2.)) * ROUND)
                    .abs()
                    .round()
                    / ROUND
                    < (start - end).abs() as f32 / 2. + 1.
            }
            // Mechanic::Cone { pos, spread, angle } => true,
            Mechanic::Puddle {
                pos,
                size,
                duration: _,
            } => Mechanic::Aoe { pos, size }.resolve(origin)?,
            _ => true,
        })
    }
}
