# Ally AI Architecture Documentation

## Overview

The ally AI system has been comprehensively refactored to be **modular**, **readable**, and **maintainable**. The logic is now split into focused, single-responsibility components with clear interfaces and well-documented behavior.

The AI supports different strength levels (WEAK, MEDIUM, STRONG) that affect behavior parameters and capabilities. Additionally, a NO_ALLY option exists that completely disables ally spawning for solo mode without AI assistance.

---

## AI Strength Levels

The ally AI behavior varies based on the selected difficulty level:

### WEAK AI
- **Speed**: 70% of normal speed (140.0f units/second)
- **Shooting**: Standard shooting interval (0.5s)
- **Special Behavior**: 30% chance to enter idle state for 2-4 seconds every 3 seconds
- **Purpose**: Easier gameplay, less aggressive ally

### MEDIUM AI (Default)
- **Speed**: Normal speed (200.0f units/second)
- **Shooting**: Standard shooting interval (0.5s)
- **Special Behavior**: None
- **Purpose**: Balanced gameplay experience

### STRONG AI
- **Speed**: 120% of normal speed (240.0f units/second)
- **Shooting**: 40% faster shooting interval (0.2s)
- **Special Behavior**: Uses charge shots when 2+ enemies are clustered within 50 pixels vertically
- **Purpose**: Challenging gameplay, very aggressive ally

### NO_ALLY
- **Behavior**: No ally entity is spawned at all
- **Purpose**: Pure solo play without AI assistance
- **Implementation**: Lobby skips ally spawning when `AIDifficulty::NO_ALLY` is selected

**Note**: The client sends `AIDifficulty` (WEAK, MEDIUM, STRONG, NO_ALLY) to the server, which maps WEAK/MEDIUM/STRONG to the corresponding `AIStrength` values. NO_ALLY results in no ally being spawned.

---

## Architecture

## Architecture

### Layered Design

```
┌─────────────────────────────────────────┐
│        AllySystem (ECS)                 │
│  (Manages AI lifecycle per entity)      │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│        AllyAI (Main Controller)         │
│  (Orchestrates all behaviors)           │
└──┬──────────────┬──────────────┬────────┘
   │              │              │
   ▼              ▼              ▼
Perception    Behavior      Constraints
(detect)     (decide)       (apply)
```

---

## File Structure

### Header Files (Interfaces)

#### 1. **`include/ai/AllyAI.hpp`** - Main Controller
- **Purpose**: Orchestrates the complete AI update cycle
- **Key Class**: `AllyAI`
- **Methods**:
  - `update()` - Main frame update
  - `reset()` - Reset AI state for reuse
- **Behavior**: Coordinates behavior systems and perception

#### 2. **`include/ai/AllyBehavior.hpp`** - Decision Systems
- **Purpose**: Handles AI decision-making for movement, shooting, and animation
- **Key Classes**:
  - `MovementBehavior` - Controls movement velocities
  - `ShootingBehavior` - Manages shooting logic
  - `AnimationBehavior` - Updates sprite animations
- **Characteristics**: Modular, state-tracked, reusable

#### 3. **`include/ai/AllyPerception.hpp`** - Awareness Systems
- **Purpose**: Handles environment sensing and threat avoidance
- **Key Classes**:
  - `EnemyPerception` - Finds nearest enemy on screen
  - `ObstacleAvoidance` - Intelligent predictive avoidance
  - `ViewportConstraint` - Boundary enforcement
- **Features**:
  - Predictive positioning (0.5s lookahead)
  - Priority-based threat evaluation
  - Emergency response system

#### 4. **`include/ai/AllyAIUtility.hpp`** - Utilities & Constants
- **Purpose**: Shared math functions and configuration constants
- **Sections**:
  - **AIStrength enum**: Defines WEAK, MEDIUM, STRONG difficulty levels
  - **CONSTANTS**: All configurable parameters in one place, with multipliers for different AI strengths
  - **MATH_UTILITIES**: Distance, direction, vector operations
  - **COMPONENT_QUERIES**: Efficient entity component access
- **Benefits**: Easy tuning, no magic numbers, reusable math, difficulty-based behavior

### Implementation Files

- **`src/ai/AllyAI.cpp`** - Main controller implementation (30 lines)
- **`src/ai/AllyBehavior.cpp`** - Behavior systems implementation (130 lines)
- **`src/ai/AllyPerception.cpp`** - Perception & avoidance implementation (220 lines)
- **`src/ai/AllyAIUtility.cpp`** - Utility functions implementation (50 lines)

---

## Component Responsibilities

### 1. MovementBehavior
**Manages ally movement decisions**

```cpp
// Handles:
// - Vertical alignment with target (enemy or player)
// - Periodic horizontal direction changes (random but natural)
// - Timer-based state management

// Internal Timer: m_horizontalTimer
// - Tracks when to change direction
// - Resets every 2 seconds (HORIZONTAL_CHANGE_INTERVAL)

// Internal State: m_currentXDirection
// - Stores current horizontal direction: -1.0, 0.0, or 1.0
// - Updated randomly each interval
```

**Key Methods**:
- `update()` - Calculates and applies velocities
- `updateHorizontalDirection()` - Random direction selection
- `calculateVerticalVelocity()` - Y-axis alignment logic
- `calculateHorizontalVelocity()` - X-axis movement logic

---

### 2. ShootingBehavior
**Controls when and how the ally shoots**

```cpp
// Handles:
// - Shooting timer management
// - Y-alignment check for accuracy
// - Shoot event emission

// Internal Timer: m_shootingTimer
// - Accumulates over time
// - Resets when shooting condition met
// - Interval: 0.5 seconds (SHOOTING_INTERVAL)

// Conditions to Shoot:
// 1. Ally and target Y-positions within 50 pixels (VERTICAL_ALIGNMENT_THRESHOLD)
// 2. Shooting timer >= interval
```

**Key Methods**:
- `update()` - Periodic update with event emission
- `isAlignedForShooting()` - Checks Y-alignment
- `shoot()` - Emits ShootEvent toward target

---

### 3. AnimationBehavior
**Updates sprite frames based on movement**

```cpp
// Handles:
// - Frame selection based on velocity direction
// - Static utility (no state needed)

// Animation Logic:
// - Up (dy < -10.0):     Frame 4
// - Down (dy > 10.0):    Frame 0
// - Neutral (other):     Frame 2

// Pure Function Design:
// - Stateless, deterministic
// - Can be called from anywhere
```

**Key Methods**:
- `update()` - Selects and applies frame
- `selectAnimationFrame()` - Direction-based frame selection

---

### 4. EnemyPerception
**Detects and tracks enemies**

```cpp
// Handles:
// - Scanning all enemies on screen
// - Distance calculation
// - Viewport boundary checking

// Algorithm:
// 1. Get all entities with Pattern component (enemies)
// 2. Filter: Must be alive and in viewport
// 3. Calculate distance to ally
// 4. Return closest enemy (or 0 if none found)

// Viewport Filtering:
// - Prevents detection of off-screen enemies
// - Uses player viewport dimensions
// - Default fallback: 1920x1080
```

**Key Methods**:
- `findNearestEnemy()` - Main detection function
- `isWithinViewportBounds()` - Boundary check
- `getViewportBounds()` - Dimension retrieval

---

### 5. ObstacleAvoidance
**Intelligent predictive collision avoidance with multi-threat handling and center-preference**

```cpp
// Handles:
// - Enemy threat evaluation (multi-threat support)
// - Projectile threat evaluation (multi-threat support)
// - Predictive collision detection
// - Priority-based threat response
// - Center-preference blending (avoids screen edges)

// Avoidance Algorithm (Enhanced Phase 3):
// 1. Get ally's current radius
// 2. For each enemy:
//    - Predict position (0.5s ahead)
//    - Calculate distance and threat weight
//    - Accumulate avoidance force
//    - Increment threat counter if threat detected
// 3. For each projectile (not owned by ally):
//    - Same process, higher priority weight
//    - Increment threat counter if threat detected
// 4. If threats detected:
//    - Normalize combined avoidance vector
//    - Apply center-preference blending (see below)
//    - Override velocity with escape direction
//    - Scale speed by threat level

// Center-Preference Blending (NEW - Phase 3):
// Purpose: Prevents ally from getting stuck at screen edges
// 
// Logic:
// - IF avoidance force is weak (magnitude < 0.2) OR multiple threats exist:
//   - Blend with movement toward screen center
//   - Weighting depends on threat situation:
//     * Weak avoidance (no clear escape): 80% center, 20% avoidance
//     * 3+ simultaneous threats: 50% center, 50% avoidance
//     * Normal case (1-2 threats): 30% center, 70% avoidance
// - Result: Ally prefers exiting to center but still respects immediate threats

// Threat Tracking (NEW - Phase 3):
// - threatCount field in AvoidanceState tracks number of active threats
// - Enables adaptive decision-making for multiple enemies
// - Allows center-preference strength to scale with threat complexity

// Threat Levels:
// - Emergency: distance <= 40px → 3x speed multiplier
// - High: weight > 1.5 → 3x speed multiplier  
// - Normal: other cases → 2x speed multiplier

// Weight Calculation Factors:
// - Distance from threat (closer = higher)
// - Object size (larger = higher weight)
// - Threat type (projectiles = 2x enemies)
// - Proximity multiplier (emergency threats *= 3)
```

**Key Methods**:
- `update()` - Main avoidance loop
- `evaluateEnemyThreats()` - Enemy threat analysis
- `evaluateProjectileThreats()` - Projectile threat analysis
- `applyAvoidanceForce()` - Velocity override
- `calculateThreatWeight()` - Weight computation
- `calculateAvoidanceDirection()` - Escape direction

---

### 6. ViewportConstraint
**Enforces screen boundary constraints**

```cpp
// Handles:
// - Position clamping
// - Collider-aware boundaries
// - Viewport-relative positioning

// Constraints:
// - X: 0 to (viewport_width / 3) - ally_width
//   Keeps ally in left 1/3 of screen
// - Y: 0 to viewport_height - ally_height
//   Keeps ally fully visible vertically

// Collider Types:
// - BOX: Use width/height directly
// - CIRCLE: Use diameter (radius * 2)
```

**Key Methods**:
- `constrainToViewport()` - Main constraint application
- `getAllySize()` - Dimension extraction
- `calculateMaxBounds()` - Boundary calculation

---

## Update Flow (Priority Order)

When `AllyAI::update()` is called:

```
1. VALIDATION
   └─ Check if entity is alive and valid
   └─ Check if in solo mode (1 player)

2. PERCEPTION
   └─ Find nearest enemy (EnemyPerception)
   └─ Select target (enemy or player)

3. MOVEMENT
   └─ Calculate movement toward target
   └─ Vertical: Align with target Y
   └─ Horizontal: Random but natural

4. SHOOTING (if enemy detected)
   └─ Check Y-alignment
   └─ Emit shoot event if aligned

5. OBSTACLE AVOIDANCE (Highest Priority)
   └─ Evaluate enemy threats
   └─ Evaluate projectile threats
   └─ Override velocity if threats detected
   └─ Emergency response if critical

6. ANIMATION
   └─ Update sprite frame based on final velocity

7. CONSTRAINTS
   └─ Clamp position to viewport bounds
   └─ Ensure ally stays on screen
```

---

## Configuration Constants

All behavior parameters are centralized in `AllyAIUtility.hpp`:

```cpp
// Movement
ALLY_SPEED = 200.0f
ALLY_SPEED_WEAK_MULTIPLIER = 0.7f      // 70% speed for WEAK AI
ALLY_SPEED_STRONG_MULTIPLIER = 1.2f    // 120% speed for STRONG AI
VERTICAL_ALIGNMENT_THRESHOLD = 50.0f
HORIZONTAL_CHANGE_INTERVAL = 2.0f
HORIZONTAL_SPEED_MULTIPLIER = 0.5f

// Shooting
SHOOTING_INTERVAL = 0.5f
SHOOTING_INTERVAL_STRONG_MULTIPLIER = 0.4f  // 40% of interval for STRONG AI

// Idle behavior (WEAK AI only)
IDLE_DURATION_MIN = 2.0f
IDLE_DURATION_MAX = 4.0f
IDLE_CHANCE = 0.3f  // 30% chance to idle

// Charge shots (STRONG AI only)
CHARGE_SHOT_ENEMY_Y_THRESHOLD = 50.0f
CHARGE_SHOT_MIN_ENEMIES = 2

// Avoidance
ENEMY_AVOID_RADIUS = 100.0f
PROJECTILE_AVOID_RADIUS = 100.0f
EMERGENCY_RADIUS = 60.0f
PREDICTION_TIME = 0.5f
AVOID_FORCE_MULTIPLIER = 2.0f
EMERGENCY_MULTIPLIER = 3.0f
HIGH_THREAT_THRESHOLD = 1.5f

// Animation
ANIMATION_FRAME_UP = 4
ANIMATION_FRAME_DOWN = 0
ANIMATION_FRAME_NEUTRAL = 2
ANIMATION_VELOCITY_THRESHOLD = 10.0f
```

**All parameters in one place for easy tuning!**

---

## Behavior Priority

### Decision Priority (What to do)
1. **Emergency Avoidance** - Escape immediate threats
2. **Enemy Pursuit** - Move toward and shoot at enemies
3. **Player Following** - Move toward player when no enemies
4. **Natural Movement** - Random horizontal wandering

### Movement Priority (How to move)
1. **Avoidance Velocity** - Highest priority override
2. **Target-Based Velocity** - Approach target
3. **Animation Direction** - Final frame selection

---

## Key Design Principles

### 1. **Single Responsibility**
Each class handles one concern:
- MovementBehavior: Movement only
- ShootingBehavior: Shooting only
- ObstacleAvoidance: Avoidance only
- etc.

### 2. **Composition Over Inheritance**
AllyAI composes behavior systems rather than inheriting from them.

### 3. **State Locality**
Each behavior owns its own timers and state:
- No global state pollution
- Easy to reset or recreate
- Thread-safe (if needed)

### 4. **Centralized Configuration**
All constants in `AllyAIUtility.hpp`:
- Easy to find parameters
- Consistent across codebase
- Single point of tuning

### 5. **Predictive AI**
Uses 0.5-second lookahead:
- Anticipates enemy positions
- More natural avoidance
- Proactive rather than reactive

### 6. **Priority-Based Avoidance**
Threats have priority:
- Projectiles > Enemies
- Close threats > distant threats
- Emergency response for critical situations

---

## Math Utilities

Located in `AllyAIUtility.hpp`:

```cpp
// Distance calculation (2D Euclidean)
float calculateDistance(x1, y1, x2, y2)
// → Returns distance, optimized sqrt

// Normalized direction vector
float calculateDirection(x1, y1, x2, y2, outX, outY)
// → Sets outX, outY to unit direction
// → Returns distance

// Vector normalization
void normalizeVector(x, y)
// → Normalizes vector in-place

// Collider radius extraction
float getColliderRadius(world, entity, defaultRadius)
// → Handles BOX and CIRCLE shapes
// → Returns effective radius

// Entity position prediction
void predictEntityPosition(world, entity, time, outX, outY)
// → Calculates future position based on velocity
// → Used for predictive avoidance

// Entity validation
bool isEntityValid(world, entity)
// → Checks: alive, has Transform, has Velocity

// NEW - Phase 3: Center direction calculation
void calculateCenterDirection(allyX, allyY, viewportW, viewportH, outX, outY)
// → Calculates unit direction toward screen center
// → Used in center-preference blending for avoidance

// NEW - Phase 3: Vector magnitude
float getVectorMagnitude(x, y)
// → Returns magnitude of 2D vector
// → Used to detect weak avoidance situations
```

---

## Phase 3 Enhancements: Multi-Threat Avoidance & Center-Preference

### Problem Statement
Previously, the ally AI could get stuck at screen edges when:
1. Multiple enemies surrounded the ally (threats canceled each other)
2. Escape routes required navigating toward safer areas
3. No clear single threat direction existed

### Solution: Adaptive Center-Preference Blending

#### Key Components Added:
1. **Threat Counting** - Track how many threats are active
2. **Avoidance Strength Detection** - Measure if avoidance is weak/unclear
3. **Adaptive Weighting** - Blend avoidance with center movement based on situation
4. **Viewport Context** - Pass screen dimensions through avoidance system

#### Implementation Details:

**New Constants** (in `AllyAIUtility.hpp`):
- `CENTER_PREFERENCE_WEIGHT = 0.3f` - Default blend ratio toward center
- `MULTI_THREAT_DEADZONE = 0.2f` - Magnitude threshold for weak avoidance

**Updated AvoidanceState**:
```cpp
struct AvoidanceState {
  float x = 0.0f, y = 0.0f;
  int threatCount = 0;  // NEW: Track simultaneous threats
};
```

**New Method** - `applyCenterPreference()`:
```cpp
// Blends avoidance direction with center movement
// Logic:
if (threatCount > 1 || avoidanceMagnitude < MULTI_THREAT_DEADZONE) {
  // Calculate direction toward screen center
  float centerX, centerY;
  calculateCenterDirection(allyX, allyY, viewportW, viewportH, centerX, centerY);
  
  // Adaptive weighting based on threat situation:
  float centerWeight = CENTER_PREFERENCE_WEIGHT; // Default 30% center
  
  if (avoidanceMagnitude < MULTI_THREAT_DEADZONE) {
    centerWeight = 0.8f;  // Weak avoidance: 80% center, 20% avoidance
  } else if (threatCount > 2) {
    centerWeight = 0.5f;  // Multiple threats: 50% center, 50% avoidance
  }
  
  // Blend the vectors
  avoidX = avoidX * (1.0f - centerWeight) + centerX * centerWeight;
  avoidY = avoidY * (1.0f - centerWeight) + centerY * centerWeight;
  normalizeVector(avoidX, avoidY);
}
```

#### Threat Evaluation Updates:
- Both `evaluateEnemyThreats()` and `evaluateProjectileThreats()` increment `state.threatCount`
- Provides accurate multi-threat awareness to decision system

#### Result:
✅ Ally escapes edge-pinning situations by preferring center movement
✅ Multiple threats handled smoothly without cancellation paralysis
✅ Behavior remains predictive and priority-based
✅ Tunable parameters for gameplay balance

---

## ECS System Integration

The `AllySystem` class acts as the ECS interface:

```cpp
class AllySystem : public ecs::ISystem {
  std::map<ecs::Entity, std::unique_ptr<ai::AllyAI>> m_allyControllers;
  
  // Creates/updates one AI controller per ally
  // Cleans up controllers for dead allies
  // Only processes in solo mode when allies are present
  // NO_ALLY difficulty prevents ally spawning entirely
};
```

**Key Features**:
- Lazy initialization (creates AI on first update)
- Automatic cleanup (removes AI for dead allies)
- Solo-mode checking (only updates when 1 player)
- Difficulty-aware (respects NO_ALLY setting by not spawning allies)

---

## Testing & Debugging

### Enable Console Output
Add logging to behavior methods:
```cpp
// In MovementBehavior::update()
std::cout << "Moving: dy=" << velocity.dy << " dx=" << velocity.dx << "\n";
```

### Adjust Parameters
Modify constants in `AllyAIUtility.hpp`:
```cpp
// Make ally faster
constexpr float ALLY_SPEED = 250.0f; // was 200.0f

// More aggressive avoidance
constexpr float ENEMY_AVOID_RADIUS = 220.0f; // was 180.0f

// Shoot more frequently
constexpr float SHOOTING_INTERVAL = 0.3f; // was 0.5f
```

### Visual Debug
Check sprite animation frame selection:
```cpp
// Current frame is in ecs::Sprite::currentFrame
// 0 = down, 2 = neutral, 4 = up
```

---

## Future Extensions

The modular design allows easy extensions:

### Add New Behavior
```cpp
// Create new class in AllyBehavior.hpp
class SpecialAbilityBehavior {
  void update(/* params */) { /* logic */ }
};

// Add to AllyAI:
class AllyAI {
  behavior::SpecialAbilityBehavior m_abilities;
};

// Call in AllyAI::updateBehaviors()
```

### Add New Threat Type
```cpp
// In ObstacleAvoidance::update():
void evaluatePowerupThreats(...) { /* ... */ }

// Then call in update():
evaluatePowerupThreats(world, allyEntity, ...);
```

### Add Different AI Personalities
```cpp
// Create AllyAIAggressive, AllyAIDefensive, etc.
// Each uses different constants:
class AllyAIAggressive : public AllyAI {
  // Override constants or behavior
};
```

---

## Summary

The refactored ally AI is now:

✅ **Modular** - Each concern is isolated  
✅ **Readable** - Clear function names and comments  
✅ **Maintainable** - Easy to find and modify behavior  
✅ **Testable** - Each component can be tested independently  
✅ **Extensible** - Simple to add new behaviors  
✅ **Well-Documented** - Comprehensive inline and header comments  
✅ **Configurable** - All parameters in one place  
✅ **Performance** - Efficient predictive calculations  
✅ **Difficulty-Aware** - Supports WEAK/MEDIUM/STRONG AI with different behaviors  
✅ **Optional** - NO_ALLY mode completely disables AI ally spawning  

The behavior remains **identical to the original** for MEDIUM difficulty, preserving all decision priorities and functionalities while providing a much cleaner, more understandable codebase with additional difficulty options.
