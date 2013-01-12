(require mzlib/math)

(define branch-scale 0.48)
(define branch-rotation 38)
(define branch-length 10)
(define branch-ratio 0.5)
(define sky-color (vector 0.1 0.1 0.2))
(define snow-color (vector 0.8 0.8 1))
(define start-time (time))

(clear)
(clear-colour sky-color)
(fog sky-color 0.01 (* 15 branch-length) (* 25 branch-length))
(ambient (vector 0.2 0.2 0.2))

(define (render-radial d maxd render-func)
    (cond
        ((>= d maxd) 0)
        (else
            (with-state
                (rotate (vector (* d (/ 360 maxd)) 0 0))
                (render-func d maxd))
            (render-radial (+ d 1) maxd render-func))))

(define (render-branch depth)
    (cond
        ((<= depth 0) 0)
        (else
            (with-state
                (scale (vector (* branch-length (min depth 1)) 1 1))
                (translate (vector 0.5 0 0))
                (rotate (vector 0 0 90))
                (draw-cube))
            (with-state
                (translate (vector (* branch-length branch-ratio) 0 0))
                (render-radial 0 2
                    (lambda (d maxd)
                        (rotate (vector 0 branch-rotation 0))
                        (scale (vector branch-scale branch-scale branch-scale))
                        (render-branch (- depth 1))))))))

(define (render-flake start)
    (render-radial 0 6
        (lambda (d maxd)
            (with-state
                (rotate (vector 0 0 90))
                (render-branch
                    (min (- (time) start (* d 0.08))
                        (flake-detail)
                        6))))))

(define (rot-pos v)
    (translate v)
    (rotate (vector 90 90 0))
    (rotate (vector (* (vx v) 6) (* (vy v) 12) (* (vz v) 12))))

(define (render-flakes flakes)
    (cond
        ((null? flakes) 0)
        (else
            (with-state
                (rot-pos (caar flakes))
                (render-flake (cdr (car flakes))))
            (render-flakes (cdr flakes)))))

(define (make-flake z-range)
    (cons
        (vector
            (* (crndf) 10 branch-length)
            (* (crndf) 10 branch-length)
            (- (* 5 branch-length) (* (rndf) z-range branch-length)))
        (time)))

(define (make-flakes n)
    (cond
        ((<= n 0) '())
        (else
            (cons (make-flake 30) (make-flakes (- n 1))))))

(define (update-flake flake v)
    (cond
        ((> (vz (car flake)) (* -25 branch-length))
            (cons (vadd (car flake) v) (cdr flake)))
        (else
            (make-flake 10))))

(define (update-flakes flakes v)
    (cond
        ((null? flakes) '())
        (else
            (cons
                (update-flake (car flakes) v)
                (update-flakes (cdr flakes) v)))))

(define (render-snow flakes v)
    (colour snow-color)
    (render-flakes flakes)
    (set! render-func
        (lambda ()
            (render-snow
                (update-flakes flakes v)
                (vlerp (vector 0.01 0.01 -0.2) v 0.01)))))

(define (render-first flakes v)
    (colour snow-color)
    (render-flakes flakes)
    (cond
        ((< (- (time) start-time) 10)
            (set! render-func
                (lambda ()
                    (render-first (update-flakes flakes v) v))))
        (else
            (set! render-func
                (lambda ()
                    (render-snow
                        (cons (car (update-flakes flakes v)) (make-flakes 99))
                        v))))))

(define (render-snow-func)
    (render-first
        (list (cons (vector 0 0 0) (time)))
        (vector 0.005 0.002 -0.02)))

(define (get-translation m)
    (vector
        (vector-ref m 12)
        (vector-ref m 13)
        (vector-ref m 14)))

(define (get-camera-position)
    (get-translation (minverse (get-camera-transform))))

(define (vlen v)
    (sqrt (vdot v v)))

(define (flake-detail)
    (max 2
        (/ (* branch-length 15)
            (vlen
                (vsub
                    (get-translation (get-transform))
                    (get-camera-position))))))

(define tree-length 7)
(define tree-scale 0.6)
(define tree-rotation 60)
(define tree-degree 5)
(define tree-ratio 0.9)
(define tree-depth 5)

(define (rndi mini maxi)
    (+  mini
        (floor (* (rndf) (- maxi mini -1)))))

(define (log-base b x)
    (/ (log x) (log b)))

(define (tree-generate)
    (set! tree-length
        (rndi 4 9))
    (set! tree-scale
        (+ (* (rndf) 0.25) 0.45))
    (set! tree-rotation
        (rndi 40 70))
    (set! tree-degree
        (rndi 3 7))
    (set! tree-depth
        (- (floor (log-base tree-degree 6000)) 0))
    (set! start-time (time))
    (wind-reset))

(define (render-tree-branch depth maxdepth)
    (cond
        ((>= depth maxdepth) 0)
        (else
            (with-state
                (apply-wind)
                (with-state
                    (if (<= (- tree-depth depth) 1) (colour (vector 0 1 0)) 0)
                    (scale (vector (* tree-length (min (- maxdepth depth) 1)) 1 1))
                    (translate (vector 0.5 0 0))
                    (rotate (vector 0 0 90))
                    (draw-cube))
                (with-state
                    (translate (vector (* tree-length tree-ratio) 0 0))
                    (render-radial 0 tree-degree
                        (lambda (d maxd)
                            (rotate (vector 0 tree-rotation 0))
                            (scale (vector tree-scale tree-scale tree-scale))
                            (render-tree-branch (+ depth 1) maxdepth))))))))

(define (render-tree)
    (colour (vector 0.3 0.1 0))
    (rotate (vector 0 0 90))
    (translate (vector (- 5) 0 0))
    (wind-update)
    (with-state
        (colour (vector 0 0.8 0))
        (rotate (vector 0 90 0))
        (scale (vector 100 100 100))
        (draw-plane))
    (let ((s (/ 4 tree-length)))
        (scale (vector s s s)))
    (rotate (vector 30 0 0))
    (render-tree-branch
        0
        (min (- (time) start-time) tree-depth)))

(define wind-start-time start-time)
(define wind-current (vector 0 0 0))
(define wind-main-target (vector 0 0 0))
(define wind-main-current wind-main-target)
(define wind-main-end-time start-time)

(define (wind-reset)
    (set! wind-start-time start-time)
    (set! wind-current (vector 0 0 0))
    (set! wind-main-target (vector 0 0 0))
    (set! wind-main-current wind-main-target)
    (set! wind-main-end-time (+ (time) 5)))

(define (wind-perturb-frequency lv)
    (lerp 3.4 1 lv))

(define (wind-update)
    (wind-main-update)
    (let ((period (/ (* pi 2) (wind-perturb-frequency (vlen wind-main-current)))))
        (set! wind-start-time
            (+  wind-start-time
                (*  period
                    (max
                        (floor (/ (- (time) wind-start-time) period))
                         0)))))
    (set! wind-current (wind)))

(define (wind-perturb v)
    (let ((lv (vlen v)))
        (vmul v
            (+  (*  (sin
                        (*  (- (time) wind-start-time)
                            (wind-perturb-frequency lv)))
                    0.08)
                1))))

(define (wind-main-rnd)
    (let ((v (vmul (srndvec) 1.5)))
        (vector (vx v) 0 (vz v))))

(define (get-azimuth v)
    (if (and (zero? (vx v)) (zero? (vy v)))
        0
        (+  (atan (vz v) (vx v))
            (* pi 2))))

(define (fix-angle1 a1 a2)
    (cond
        ((and
            (> (abs (- a1 a2)) pi)
            (< a1 a2))
            (+ a1 (* pi 2)))
        (else
            a1)))

(define (fix-angle2 a1 a2)
    (cond
        ((and
            (> (abs (- a1 a2)) pi)
            (> a1 a2))
            (+ a2 (* pi 2)))
        (else
            a2)))

(define (wind-interpolate vcur vtarget)
    (let*
        ((acur (get-azimuth vcur))
        (atarget (get-azimuth vtarget))
        (fcur (fix-angle1 acur atarget))
        (ftarget (fix-angle2 acur atarget))
        (a (lerp ftarget fcur 0.01))
        (l (lerp (vlen vtarget) (vlen vcur) 0.01)))
            (vector (* l (cos a)) 0 (* l (sin a)))))

(define (wind-main-update)
    (cond
        ((>= (time) wind-main-end-time)
            (set! wind-main-target
                (wind-main-rnd))
            (set! wind-main-end-time
                (+  (* (rndf) 15)
                    (time)
                    5)))
        (else 0))
    (set! wind-main-current
        (wind-interpolate wind-main-current wind-main-target)))

(define (wind)
    (wind-perturb wind-main-current))

(define (mx m)
    (vector (vector-ref m 0) (vector-ref m 1) (vector-ref m 2)))

(define (my m)
    (vector (vector-ref m 4) (vector-ref m 5) (vector-ref m 6)))

(define (mz m)
    (vector (vector-ref m 8) (vector-ref m 9) (vector-ref m 10)))

(define (mmake x y z t)
    (vector
        (vx x) (vy x) (vz x) 0
        (vx y) (vy y) (vz y) 0
        (vx z) (vy z) (vz z) 0
        (vx t) (vy t) (vz t) 1))

(define (vproj u v)
    (vmul u (/ (vdot u v) (vdot u u))))

(define (vsetlen v l)
    (vmul v (/ l (vlen v))))

(define (orthogonalize-y x y)
    (vsetlen (vsub y (vproj x y)) (vlen y)))

(define (orthogonalize-z x y z)
    (vsetlen (vsub z (vproj x z) (vproj y z)) (vlen z)))

(define (wind-deflect v)
    (let ((w wind-current))
        (vadd v
            (vmul w
                (*  (vlen (vcross (vnormalise v) w))
                    0.1
                    (sqrt (vlen v)))))))

(define (apply-wind)
    (let*
        ((xform (get-transform))
        (x (mx xform))
        (y (my xform))
        (z (mz xform))
        (t (get-translation xform))
        (nx (wind-deflect x))
        (ny (orthogonalize-y nx y))
        (nz (orthogonalize-z nx ny z))
        (nt t)
        (nxform (mmake nx ny nz nt))
        (mat (mmul (minverse xform) nxform))
        (euler (matrix->euler mat)))
            (rotate euler)))

(define render-func render-snow-func)

(define (render-main)
    (cond
        ((key-pressed "s")
            (set! start-time (time))
            (set! render-func render-snow-func))
        ((key-pressed "t")
            (tree-generate)
            (set! render-func render-tree)))
    (render-func))

(every-frame (render-main))

(display "Press S for snow\n")
(display "Press T for a tree\n")
