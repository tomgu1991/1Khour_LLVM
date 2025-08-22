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
  %multmp = fmul double %a, 2.000000e+00
  %multmp1 = fmul double %multmp, %b
  %addtmp = fadd double %multmp1, %calltmp
  %calltmp2 = call double @squire(double %b)
  %addtmp3 = fadd double %addtmp, %calltmp2
  ret double %addtmp3
}

define double @fib(double %x) {
entry:
  %cmptmp = fcmp ult double %x, 3.000000e+00
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %entry
  %calltmp = call double @squire(double %x)
  br label %ifcont

else:                                             ; preds = %entry
  %subtmp = fadd double %x, -1.000000e+00
  %calltmp1 = call double @fib(double %subtmp)
  %calltmp2 = call double @squire(double %x)
  %addtmp = fadd double %calltmp1, %calltmp2
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi double [ %calltmp, %then ], [ %addtmp, %else ]
  ret double %iftmp
}