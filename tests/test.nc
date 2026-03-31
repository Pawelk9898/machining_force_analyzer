; Simple side shaving test
; Stock: X(-50 to 50) Y(-50 to 50) Z(-20 to 0)
; Tool: 10mm diameter end mill

G21        ; metric
G90        ; absolute mode
S5000 M3   ; spindle on at 5000 RPM

; Move to safe position above stock
G0 X-60 Y0 Z5

; Plunge to cutting depth
G0 X-60 Y0 Z-15

; Shave 3mm from the left side of stock (X=-50 face)
; Tool center at X=-47 removes material from X=-52 to X=-42
G1 X-47 Y-60 F500
G1 X-47 Y60 F500

; Retract
G0 Z5

; Second pass slightly deeper into stock
G0 X-47 Y-60 Z-10
G1 X-44 Y-60 F500
G1 X-44 Y60 F500

; Retract to safe position
G0 Z5
G0 X-60 Y0

M5   ; spindle off
M30  ; end of program