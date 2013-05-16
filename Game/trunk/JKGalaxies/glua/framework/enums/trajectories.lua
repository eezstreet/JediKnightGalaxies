-- TR_** definitions

TR_STATIONARY = 0 	-- Dont move at all
TR_INTERPOLATE = 1	-- non-parametric, but interpolate between snapshots
TR_LINEAR = 2		-- Keep moving in a straight line
TR_LINEAR_STOP = 3 	-- Move in a straight line and stop once the target is reached
TR_NONLINEAR_STOP = 4 -- Move and stop while decelerating
TR_SINE = 5			-- value = base + sin( time / duration ) * delta
TR_GRAVITY = 6		-- Use gravity
TR_NONLINEAR_ACCEL_STOP = 7 -- JKG: Like NONLINEAR_STOP, but instead of decelerating, it accelerates
-- JKG - Bezier Curve interpolated trajectories
TR_EASEINOUT = 8 	-- Accelerate and then decelerate

TR_HARDEASEIN = 9 	-- Slowly accelerate and pick up speed rapidly
TR_SOFTEASEIN = 10 	-- Accelerate and pick up speed (equal to TR_NONLINEAR_ACCEL_STOP)

TR_HARDEASEOUT = 11 -- Start fast and come to a slow and smooth halt
TR_SOFTEASEOUT = 12 -- Start fast and come to a smooth halt (equal to TR_NONLINEAR_STOP)
