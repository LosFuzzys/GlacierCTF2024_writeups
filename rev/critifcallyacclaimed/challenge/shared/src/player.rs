use crate::point::Point;
use crate::Direction;

pub(crate) const MAX_PLAYER_HEALTH: i32 = 100;
pub(crate) const PLAYER_HEAL: i32 = (MAX_PLAYER_HEALTH as f32 * 0.8) as i32;
pub(crate) const PLAYER_BLOCK_FACTOR: f32 = 0.5;

#[cfg_attr(feature = "debug", derive(Debug))]
#[derive(Copy, Clone)]
pub struct Player {
    health: i32,
    pos: Point,
    blocking: bool,
    last_movement: Direction,
    moved: bool,
}

impl Player {
    /// Creates a new [`Player`]
    #[inline]
    pub fn new() -> Self {
        Self {
            health: MAX_PLAYER_HEALTH,
            pos: (0, 0).into(),
            blocking: false,
            last_movement: Direction::N,
            moved: false,
        }
    }

    /// Damages the player. This damage is halved if player is [`Player::blocking`]
    #[inline]
    pub fn damage(&mut self, amount: i32) {
        let amount = amount as f32;
        self.health -= (amount - (amount * self.blocking as i32 as f32 * PLAYER_BLOCK_FACTOR)).round() as i32;
        if amount > 0. {
            self.blocking = false;
        }
    }

    /// Heals the player (cannot overheal, caps at [`MAX_PLAYER_HEALTH`])
    #[inline]
    pub fn heal(&mut self) {
        self.health = (self.health + PLAYER_HEAL).min(MAX_PLAYER_HEALTH);
    }

    /// Teleports the player to the position.
    /// Does not set [`Player::moved`]
    #[inline]
    pub fn force_move_to(&mut self, p: Point) {
        self.pos.x = p.x;
        self.pos.y = p.y;
    }

    /// Moves the player by an amount relative to their current position.
    /// Does not set [`Player::moved`]
    #[inline]
    pub fn force_move_by(&mut self, p: Point) {
        self.pos.x += p.x;
        self.pos.y += p.y;
    }

    /// Moves the player by 1 in the specifid [`Direction`].
    /// Also sets [`Player::moved`] and [`Player::last_movement`]
    #[inline]
    pub fn player_move(&mut self, dir: Direction) {
        let m = match dir {
            Direction::N => (0, 1),
            Direction::S => (0, -1),
            Direction::W => (-1, 0),
            Direction::E => (1, 0),
        };
        self.pos.x += m.0;
        self.pos.y += m.1;
        self.last_movement = dir;
        self.moved = true;
    }

    /// Represents a game tick. Resets [`Player::blocking`] and [`Player::last_movement`]
    #[inline]
    pub fn tick(&mut self) {
        self.blocking = false;
        self.moved = false;
    }

    /// Sets [`Player::blocking`]
    #[inline]
    pub fn block(&mut self) {
        self.blocking = true;
    }

    /// Gets the player's position
    #[inline]
    pub fn pos(&self) -> Point {
        self.pos
    }

    /// https://www.youtube.com/watch?v=dyIilW_eBjc
    #[inline]
    pub fn kill(&mut self) {
        self.health = 0
    }

    /// For debugging purposes
    #[inline]
    pub fn force_set_moved(&mut self) {
        self.moved = true;
    }

    /// For debugging purposes
    #[inline]
    pub fn force_set_direction(&mut self, d: Direction) {
        self.last_movement = d;
    }

    /// Did we move?
    #[inline]
    pub fn did_move(&self) -> bool {
        self.moved
    }

    /// Where did we move to?
    #[inline]
    pub fn last_moved(&self) -> Direction {
        self.last_movement
    }

    #[inline]
    pub fn health(&self) -> i32 {
        self.health
    }

    /// Checks if health is greater than 0
    ///
    /// STAYIN' ALIVE
    #[inline]
    pub fn is_alive(&self) -> bool {
        self.health > 0
    }
}
