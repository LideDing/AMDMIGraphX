
2
0Placeholder*
dtype0*
shape
:
2
1Placeholder*
shape
:*
dtype0
7
assert_less_equal/LessEqual	LessEqual01*
T0
L
assert_less_equal/ConstConst*
valueB"       *
dtype0
o
assert_less_equal/AllAllassert_less_equal/LessEqualassert_less_equal/Const*

Tidx0*
	keep_dims( 
G
assert_less_equal/Assert/ConstConst*
valueB B *
dtype0
~
 assert_less_equal/Assert/Const_1Const*
dtype0*F
value=B; B5Condition x <= y did not hold element-wise:x (0:0) = 
S
 assert_less_equal/Assert/Const_2Const*
valueB B
y (1:0) = *
dtype0
l
+assert_less_equal/Assert/AssertGuard/SwitchSwitchassert_less_equal/Allassert_less_equal/All*
T0

q
-assert_less_equal/Assert/AssertGuard/switch_tIdentity-assert_less_equal/Assert/AssertGuard/Switch:1*
T0

o
-assert_less_equal/Assert/AssertGuard/switch_fIdentity+assert_less_equal/Assert/AssertGuard/Switch*
T0

X
,assert_less_equal/Assert/AssertGuard/pred_idIdentityassert_less_equal/All*
T0

a
)assert_less_equal/Assert/AssertGuard/NoOpNoOp.^assert_less_equal/Assert/AssertGuard/switch_t
�
7assert_less_equal/Assert/AssertGuard/control_dependencyIdentity-assert_less_equal/Assert/AssertGuard/switch_t*^assert_less_equal/Assert/AssertGuard/NoOp*
T0
*@
_class6
42loc:@assert_less_equal/Assert/AssertGuard/switch_t
�
2assert_less_equal/Assert/AssertGuard/Assert/data_0Const.^assert_less_equal/Assert/AssertGuard/switch_f*
valueB B *
dtype0
�
2assert_less_equal/Assert/AssertGuard/Assert/data_1Const.^assert_less_equal/Assert/AssertGuard/switch_f*F
value=B; B5Condition x <= y did not hold element-wise:x (0:0) = *
dtype0
�
2assert_less_equal/Assert/AssertGuard/Assert/data_3Const.^assert_less_equal/Assert/AssertGuard/switch_f*
valueB B
y (1:0) = *
dtype0
�
+assert_less_equal/Assert/AssertGuard/AssertAssert2assert_less_equal/Assert/AssertGuard/Assert/Switch2assert_less_equal/Assert/AssertGuard/Assert/data_02assert_less_equal/Assert/AssertGuard/Assert/data_14assert_less_equal/Assert/AssertGuard/Assert/Switch_12assert_less_equal/Assert/AssertGuard/Assert/data_34assert_less_equal/Assert/AssertGuard/Assert/Switch_2*
T	
2*
	summarize
�
2assert_less_equal/Assert/AssertGuard/Assert/SwitchSwitchassert_less_equal/All,assert_less_equal/Assert/AssertGuard/pred_id*
T0
*(
_class
loc:@assert_less_equal/All
�
4assert_less_equal/Assert/AssertGuard/Assert/Switch_1Switch0,assert_less_equal/Assert/AssertGuard/pred_id*
T0*
_class

loc:@0
�
4assert_less_equal/Assert/AssertGuard/Assert/Switch_2Switch1,assert_less_equal/Assert/AssertGuard/pred_id*
T0*
_class

loc:@1
�
9assert_less_equal/Assert/AssertGuard/control_dependency_1Identity-assert_less_equal/Assert/AssertGuard/switch_f,^assert_less_equal/Assert/AssertGuard/Assert*
T0
*@
_class6
42loc:@assert_less_equal/Assert/AssertGuard/switch_f
�
*assert_less_equal/Assert/AssertGuard/MergeMerge9assert_less_equal/Assert/AssertGuard/control_dependency_17assert_less_equal/Assert/AssertGuard/control_dependency*
N*
T0

G
add1Add01+^assert_less_equal/Assert/AssertGuard/Merge*
T0"&