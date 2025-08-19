; ModuleID = 'my cool jit'
source_filename = "my cool jit"

define double @squire(double %a) {
entry:
  %multmp = fmul double %a, %a
  ret double %multmp
}

define double @foo(double %a, double %b) {
entry:
  %calltmp = call double @squire(double %a)
  %multmp = fmul double 2.000000e+00, %a
  %multmp1 = fmul double %multmp, %b
  %addtmp = fadd double %calltmp, %multmp1
  %calltmp2 = call double @squire(double %b)
  %addtmp3 = fadd double %addtmp, %calltmp2
  ret double %addtmp3
}