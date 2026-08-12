# Differential Robot Odometry and Cascaded PI Control Guide

## 1. Dead Reckoning (Odometry) Basics
Dead reckoning for a differential robot involves tracking wheel rotations over small time intervals ($\Delta t$) and integrating them to update the robot's global position.

### Why Dead Reckoning Drifts
* **Wheel Slip:** Tires slide on smooth timber floors; encoders count rotations, but the robot does not move.
* **Surface Bumps:** Micro-variations cause wheels to lose traction or turn without forward progress.
* **Mechanical Errors:** Discrepancies in wheel diameter or track width cause long-term trajectory bending.
* **Compass Drift:** Angle tracking errors accumulate over time.

### The Kinematic Update Algorithm
Run this loop at a high frequency (e.g., 50Hz to 100Hz):

1. **Calculate Linear Distance Moved by Each Wheel:**
   $$s_L = \frac{\text{Left Ticks}}{\text{Total Ticks Per Turn}} \times (2 \pi R)$$
   $$s_R = \frac{\text{Right Ticks}}{\text{Total Ticks Per Turn}} \times (2 \pi R)$$
   *(where $R$ is the wheel radius in metres)*

2. **Calculate Total Robot Movement & Heading Change:**
   $$s = \frac{s_R + s_L}{2}$$
   $$\Delta \theta = \frac{s_R - s_L}{L}$$
   *(where $L$ is the track width in metres and $\Delta \theta$ is in radians)*

3. **Compute Global Delta-X and Delta-Y Vectors:**
   $$\Delta x = s \times \cos\left(\theta + \frac{\Delta \theta}{2}\right)$$
   $$\Delta y = s \times \sin\left(\theta + \frac{\Delta \theta}{2}\right)$$
   *(Adding $\frac{\Delta \theta}{2}$ accounts for the arc curvature during the turn)*

4. **Update Global Coordinates:**
   $$x_{\text{new}} = x_{\text{old}} + \Delta x$$
   $$y_{\text{new}} = y_{\text{old}} + \Delta y$$
   $$\theta_{\text{new}} = \theta_{\text{old}} + \Delta \theta$$

---

## 2. Solving Motor Asymmetry with Cascaded PI Control
When one motor runs backward relative to the other, directional friction and internal motor efficiency differences can cause up to a 10% PWM discrepancy for the same speed.

Instead of baking this bias into high-level steering equations, use a **Cascaded Control Architecture**. The high-level navigation code outputs global velocities, which are split into individual wheel target velocities. Independent wheel PI controllers then dynamically compute the unique PWM required for each side, automatically neutralizing the asymmetry.

## High-Level Split (Forward Kinematics)
Convert desired robot linear speed ($v$) and angular turning rate ($\omega$) into target wheel speeds ($v_L^*$ and $v_R^*$):
$$v_L^* = v - \frac{\omega \cdot L}{2}$$
$$v_R^* = v + \frac{\omega \cdot L}{2}$$

### Dynamic Wheel PI Loop
Compute actual current wheel velocities ($v_{\text{wheel}}$) using encoder updates over $\Delta t$:
$$v_{\text{wheel}} = \frac{\text{Ticks inside current interval}}{\text{Total Ticks Per Turn}} \times \frac{2\pi R}{\Delta t}$$

Run the parallel PI loop for each wheel:
$$\text{Error}_L = v_L^* - v_L$$
$$\text{Integral}_L = \text{Integral}_L + (\text{Error}_L \times \Delta t)$$
$$\text{PWM}_L = (K_p \times \text{Error}_L) + (K_i \times \text{Integral}_L)$$

---

## 3. Feedforward (FF) Speed Profiling
To prevent the PI loop from lagging during rapid acceleration across varying speeds, inject a Feedforward term based on measured physical characteristics.

1. Prop the robot up so wheels spin freely.
2. Send a baseline PWM (e.g., `100`) to the Left motor (Forward) and note steady-state velocity ($v_{\text{test}, L}$).
3. Send a baseline PWM (e.g., `100`) to the Right motor (Backward) and note steady-state velocity ($v_{\text{test}, R}$).
4. Calculate feedforward scale factors:
   $$K_{ff, L} = \frac{100}{v_{\text{test}, L}} \quad \text{and} \quad K_{ff, R} = \frac{100}{v_{\text{test}, R}}$$
   *(The less efficient motor will yield a higher $K_{ff}$ value, boosting its baseline PWM)*

### Combining PI + Feedforward:
$$\text{PWM}_L = \left(K_p \times \text{Error}_L + K_i \times \text{Integral}_L\right) + \left(K_{ff, L} \times v_L^*\right)$$
$$\text{PWM}_R = \left(K_p \times \text{Error}_R + K_i \times \text{Integral}_R\right) + \left(K_{ff, R} \times v_R^*\right)$$

---

## 4. Heuristic Loop Tuning Guide

### Setup Configuration
* Prop up the robot so the wheels can safely spin freely without driving off your workspace.
* Format your code variables strictly in standard units: target/actual speeds in **m/s**, time variables in **seconds**.
* Direct target and actual wheel speeds to a serial monitor or visual plotter.

### Step-by-Step Tuning Protocol

#### Step 1: Tune Proportional Gain ($K_p$)
The proportional term provides the immediate response kick.
1. Initialize values to $K_i = 0$ and $K_p = 0$.
2. Inject a controlled step-change into the target speed (e.g., jump from 0 m/s to 0.2 m/s).
3. Gradually increase $K_p$ (e.g., 1.0, 2.0, 5.0).
4. Watch behavior:
    * **Under-tuned:** Wheel accelerates sluggishly and plateaus far below the target line.
    * **Over-tuned:** Wheel vibrates, produces high-pitched motor wine, or oscillates wildly over the target line.
    * **Optimal:** Wheel spins up swiftly and stabilizes around **70% to 90%** of the target speed with a fixed, stable steady-state error gap.

#### Step 2: Tune Integral Gain ($K_i$)
The integral term evaluates history over time to zero out steady-state error and counter time-varying biases (such as battery depletion or heat friction).
1. Hold the functional $K_p$ value discovered in Step 1 constant.
2. Introduce a low $K_i$ start point (e.g., $K_i = 0.5$).
3. Increment $K_i$ gradually while executing target speed steps.
4. Watch behavior:
    * **Under-tuned:** Speed requires multiple seconds to climb up to meet the target line.
    * **Over-tuned:** Actual speed aggressively overshoots the target line, oscillating multiple times before stabilizing.
    * **Optimal:** Speed rises smoothly along a clean curve, locking firmly onto the target line within a fraction of a second.

### Crucial Engineering Safeguards

#### 1. Integral Anti-Windup Clamping
To prevent the integral accumulator from winding up to astronomical levels when the wheels encounter mechanical friction or resistance, enforce saturation limits:
```cpp
float max_integral = 150.0; // Restrict value within operational PWM limits (e.g., 255)
integral_L += error_L * delta_t;

if (integral_L > max_integral)  integral_L = max_integral;
if (integral_L < -max_integral) integral_L = -max_integral;
```

#### 2. Real-World Adjustments
Free-spinning bench testing requires minimal mechanical torque. When the robot is positioned down on the timber floor, the physical load may introduce minor sluggishness. Scale both $K_p$ and $K_i$ up uniformly by **10% to 20%** from your bench baseline to optimize real-world performance.
If you want to continue planning your code layout, let me know if you would like to look at a complete object-oriented C++ or Python structure that links all of these modular mathematical components together!