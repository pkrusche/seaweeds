
namespace {
	const char __compnet_stage_2d_cal_desc_tech0_pass0[] = "il_ps_2_0\n"
		"dcl_literal l0,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_literal l1,0x00000001,0x00000001,0x00000001,0x00000001\n"
		"dcl_literal l2,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF\n"
		"dcl_literal l3,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF\n"
		"dcl_literal l4,0x7F800000,0x7F800000,0x7F800000,0x7F800000\n"
		"dcl_literal l5,0x80000000,0x80000000,0x80000000,0x80000000\n"
		"dcl_literal l6,0x3E9A209B,0x3E9A209B,0x3E9A209B,0x3E9A209B\n"
		"dcl_literal l7,0x3F317218,0x3F317218,0x3F317218,0x3F317218\n"
		"dcl_literal l8,0x40490FDB,0x40490FDB,0x40490FDB,0x40490FDB\n"
		"dcl_literal l9,0x3FC90FDB,0x3FC90FDB,0x3FC90FDB,0x3FC90FDB\n"
		"dcl_literal l10,0x00000003,0x00000003,0x00000003,0x00000003\n"
		"dcl_literal l11,0x00000002,0x00000002,0x00000002,0x00000002\n"
		"dcl_literal l12,0x00000001,0x00000001,0x00000001,0x00000001\n"
		"dcl_literal l13,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_output_usage(generic) o0.xyzw\n"
		"dcl_resource_id(0)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_resource_id(1)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_resource_id(2)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_input_position_interp(linear_noperspective) v0.xy__\n"
		"dcl_literal l14,0x3F000000,0x3F000000,0x3F000000,0x3F000000\n"
		"dcl_literal l15,0x3F000000,0x3F000000,0x3F000000,0x3F000000\n"
		"dcl_cb cb0[4]\n"
		"mov r331.xy__,v0.xyzw\n"
		"mov r269.x___,cb0[l0.x + 0].x000\n"
		"mov r270.xyz_,cb0[l0.x + 0].yzw0\n"
		"mov r271.x___,cb0[l0.x + 1].x000\n"
		"mov r272.xyz_,cb0[l0.x + 1].yzw0\n"
		"mov r273.x___,cb0[l0.x + 2].x000\n"
		"mov r274.xyz_,cb0[l0.x + 2].yzw0\n"
		"mov r275.x___,cb0[l0.x + 3].x000\n"
		"mov r276.xyz_,cb0[l0.x + 3].yzw0\n"
		"call 38 \n"
		"call 0 \n"
		"endmain\n"
		"\n"
		"func 0\n"
		"mov o0.xyzw,r330.xyzw\n"
		"ret\n"
		"\n"
		"func 20\n"
		"ieq r0.x___,r144.x000,l0.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(0)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"ieq r0.x___,r144.x000,l1.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(1)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"ieq r0.x___,r144.x000,l11.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(2)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"mov r147.x___,r146.x000\n"
		"mov r143.x___,r147.x000\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"func 37\n"
		"mov r340.x___,r285.y000\n"
		"mov r286.x___,r340.x000\n"
		"mov r287.x___,r285.x000\n"
		"ige r288.x___,r286.x000,r279.x000\n"
		"iadd r289.x___,r279.x000,r280.x000\n"
		"ilt r290.x___,r286.x000,r289.x000\n"
		"and r291.x___,r288.x000,r290.x000\n"
		"if_logicalnz r291.x000\n"
		"iadd r297.x___,r280.x000,r286_neg(xyzw).x000\n"
		"iadd r298.x___,r297.x000,r279.x000\n"
		"iadd r299.x___,r298.x000,l12_neg(xyzw).x000\n"
		"ishr r300.x___,r299.x000,l12.x000\n"
		"mov r292.x___,r300.x000\n"
		"iadd r301.x___,r281.x000,r292.x000\n"
		"itof r341.x___,r301.x000\n"
		"mov r302.x___,r341.x000\n"
		"itof r342.x___,r287.x000\n"
		"mov r302._y__,r342.0x00\n"
		"mov r144.x___,r277.x000\n"
		"mov r145.xy__,r302.xy00\n"
		"call 20 \n"
		"mov r343.x___,r143.x000\n"
		"mov r303.x___,r343.x000\n"
		"mov r293.x___,r303.x000\n"
		"iadd r304.x___,r282.x000,r292_neg(xyzw).x000\n"
		"itof r344.x___,r304.x000\n"
		"mov r305.x___,r344.x000\n"
		"mov r305._y__,l0.0x00\n"
		"mov r144.x___,r278.x000\n"
		"mov r145.xy__,r305.xy00\n"
		"call 20 \n"
		"mov r345.x___,r143.x000\n"
		"mov r306.x___,r345.x000\n"
		"mov r294.x___,r306.x000\n"
		"ieq r307.x___,r293.x000,r294.x000\n"
		"cmov_logical r308.x___,r307.x000,l12.x000,l13.x000\n"
		"mov r295.x___,r308.x000\n"
		"iadd r309.x___,r286.x000,r279_neg(xyzw).x000\n"
		"and r310.x___,r309.x000,l12.x000\n"
		"ieq r311.x___,r310.x000,l13.x000\n"
		"cmov_logical r312.x___,r311.x000,l12.x000,l13.x000\n"
		"mov r296.x___,r312.x000\n"
		"ieq r313.x___,r295.x000,l13.x000\n"
		"if_logicalnz r313.x000\n"
		"iadd r318.x___,r286.x000,r279_neg(xyzw).x000\n"
		"inot r319.x___,l12.x000\n"
		"and r320.x___,r318.x000,r319.x000\n"
		"iadd r321.x___,r320.x000,r279.x000\n"
		"mov r314.x___,r321.x000\n"
		"iadd r322.x___,r314.x000,l12.x000\n"
		"mov r315.x___,r322.x000\n"
		"itof r346.x___,r287.x000\n"
		"mov r323.x___,r346.x000\n"
		"itof r347.x___,r314.x000\n"
		"mov r323._y__,r347.0x00\n"
		"mov r144.x___,r283.x000\n"
		"mov r145.xy__,r323.xy00\n"
		"call 20 \n"
		"mov r348.x___,r143.x000\n"
		"mov r316.x___,r348.x000\n"
		"itof r349.x___,r287.x000\n"
		"mov r324.x___,r349.x000\n"
		"itof r350.x___,r315.x000\n"
		"mov r324._y__,r350.0x00\n"
		"mov r144.x___,r283.x000\n"
		"mov r145.xy__,r324.xy00\n"
		"call 20 \n"
		"mov r351.x___,r143.x000\n"
		"mov r317.x___,r351.x000\n"
		"ieq r325.x___,r296.x000,l13.x000\n"
		"if_logicalnz r325.x000\n"
		"imin r326.x___,r316.x000,r317.x000\n"
		"mov r284.x___,r326.x000\n"
		"else\n"
		"imax r327.x___,r316.x000,r317.x000\n"
		"mov r284.x___,r327.x000\n"
		"endif\n"
		"else\n"
		"itof r352.x___,r287.x000\n"
		"mov r328.x___,r352.x000\n"
		"itof r353.x___,r286.x000\n"
		"mov r328._y__,r353.0x00\n"
		"mov r144.x___,r283.x000\n"
		"mov r145.xy__,r328.xy00\n"
		"call 20 \n"
		"mov r354.x___,r143.x000\n"
		"mov r284.x___,r354.x000\n"
		"endif\n"
		"else\n"
		"itof r355.x___,r287.x000\n"
		"mov r329.x___,r355.x000\n"
		"itof r356.x___,r286.x000\n"
		"mov r329._y__,r356.0x00\n"
		"mov r144.x___,r283.x000\n"
		"mov r145.xy__,r329.xy00\n"
		"call 20 \n"
		"mov r357.x___,r143.x000\n"
		"mov r284.x___,r357.x000\n"
		"endif\n"
		"ret\n"
		"\n"
		"func 38\n"
		"mov r335.x___,l14.x000\n"
		"mov r335._y__,l15.0x00\n"
		"sub r336.xy__,r331.xy00,r335.xy00\n"
		"mov r358.xyzw,r336.xy00\n"
		"mov r334.xyzw,r358.xyzw\n"
		"mov r277.x___,l0.x000\n"
		"mov r278.x___,l1.x000\n"
		"mov r279.x___,r269.x000\n"
		"mov r280.x___,r271.x000\n"
		"mov r281.x___,r273.x000\n"
		"mov r282.x___,r275.x000\n"
		"mov r283.x___,l11.x000\n"
		"ftoi r359.xyzw,r334.xyzw\n"
		"mov r285.xyzw,r359.xyzw\n"
		"call 37 \n"
		"mov r333.x___,r284.x000\n"
		"mov r337.x___,r333.x000\n"
		"mov r338.x___,r337.x000\n"
		"mov r338._y__,l13.0x00\n"
		"mov r338.__z_,l13.00x0\n"
		"mov r338.___w,l13.000x\n"
		"mov r332.xyzw,r338.xyzw\n"
		"mov r330.xyzw,r332.xyzw\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"end\n"
		"";

	const char __compnet_stage_2d_cal_desc_tech1_pass0[] = "il_ps_2_0\n"
		"dcl_literal l0,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_literal l1,0x00000001,0x00000001,0x00000001,0x00000001\n"
		"dcl_literal l2,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF\n"
		"dcl_literal l3,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF\n"
		"dcl_literal l4,0x7F800000,0x7F800000,0x7F800000,0x7F800000\n"
		"dcl_literal l5,0x80000000,0x80000000,0x80000000,0x80000000\n"
		"dcl_literal l6,0x3E9A209B,0x3E9A209B,0x3E9A209B,0x3E9A209B\n"
		"dcl_literal l7,0x3F317218,0x3F317218,0x3F317218,0x3F317218\n"
		"dcl_literal l8,0x40490FDB,0x40490FDB,0x40490FDB,0x40490FDB\n"
		"dcl_literal l9,0x3FC90FDB,0x3FC90FDB,0x3FC90FDB,0x3FC90FDB\n"
		"dcl_literal l10,0x00000003,0x00000003,0x00000003,0x00000003\n"
		"dcl_literal l11,0x00000002,0x00000002,0x00000002,0x00000002\n"
		"dcl_literal l12,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_literal l13,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_literal l14,0x00000001,0x00000001,0x00000001,0x00000001\n"
		"dcl_literal l15,0x00000000,0x00000000,0x00000000,0x00000000\n"
		"dcl_literal l16,0x3F000000,0x3F000000,0x3F000000,0x3F000000\n"
		"dcl_literal l17,0x3F000000,0x3F000000,0x3F000000,0x3F000000\n"
		"dcl_output_usage(generic) o0.xyzw\n"
		"dcl_resource_id(0)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_resource_id(1)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_resource_id(2)_type(2d,unnorm)_fmtx(float)_fmty(float)_fmtz(float)_fmtw(float)\n"
		"dcl_input_position_interp(linear_noperspective) v0.xy__\n"
		"dcl_cb cb0[12]\n"
		"mov r417.xy__,v0.xyzw\n"
		"mov r342.xyzw,cb0[l0.x + 0].xyzw\n"
		"mov r343.xyzw,cb0[l0.x + 1].xyzw\n"
		"mov r344.xyzw,cb0[l0.x + 2].xyzw\n"
		"mov r345.xyzw,cb0[l0.x + 3].xyzw\n"
		"mov r346.x___,cb0[l0.x + 4].x000\n"
		"mov r347.xyz_,cb0[l0.x + 4].yzw0\n"
		"mov r348.x___,cb0[l0.x + 5].x000\n"
		"mov r349.xyz_,cb0[l0.x + 5].yzw0\n"
		"mov r350.x___,cb0[l0.x + 6].x000\n"
		"mov r351.xyz_,cb0[l0.x + 6].yzw0\n"
		"mov r352.x___,cb0[l0.x + 7].x000\n"
		"mov r353.xyz_,cb0[l0.x + 7].yzw0\n"
		"mov r354.xyzw,cb0[l0.x + 8].xyzw\n"
		"mov r355.xyzw,cb0[l0.x + 9].xyzw\n"
		"mov r356.xyzw,cb0[l0.x + 10].xyzw\n"
		"mov r357.xyzw,cb0[l0.x + 11].xyzw\n"
		"call 42 \n"
		"call 0 \n"
		"endmain\n"
		"\n"
		"func 0\n"
		"mov o0.xyzw,r416.xyzw\n"
		"ret\n"
		"\n"
		"func 20\n"
		"ieq r0.x___,r144.x000,l0.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(0)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"ieq r0.x___,r144.x000,l1.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(1)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"ieq r0.x___,r144.x000,l11.x000\n"
		"if_logicalnz r0.x000\n"
		"sample_resource(2)_sampler(0) r146.xyzw,r145.xy00\n"
		"endif\n"
		"mov r147.x___,r146.x000\n"
		"mov r143.x___,r147.x000\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"func 37\n"
		"mov r277.x___,l12.x000\n"
		"mov r277._y__,l12.0x00\n"
		"mov r277.__z_,l12.00x0\n"
		"mov r277.___w,l12.000x\n"
		"mov r273.xyzw,r277.xyzw\n"
		"ftoi r425.xy__,r270.xy00\n"
		"mov r278.xy__,r425.xy00\n"
		"mov r274.xy__,r278.xy00\n"
		"mov r426.x___,r274.y000\n"
		"imul r279.x___,r426.x000,r272.x000\n"
		"iadd r280.x___,r279.x000,r274.x000\n"
		"mov r275.x___,r280.x000\n"
		"mov r429.x___,r275.x000\n"
		"imax r427.x___,r275.x000,r429_neg(xyzw).x000\n"
		"mov r430.x___,r271.x000\n"
		"imax r428.x___,r271.x000,r430_neg(xyzw).x000\n"
		"udiv r431.x___,r427.x000,r428.x000\n"
		"inegate r432.x___,r431.x000\n"
		"ixor r433.x___,r275.x000,r271.x000\n"
		"and r433.x___,r433.x000,l5.x000\n"
		"cmov_logical r281.x___,r433.x000,r432.x000,r431.x000\n"
		"mov r276.x___,r281.x000\n"
		"imul r282.x___,r276.x000,r271.x000\n"
		"iadd r283.x___,r275.x000,r282_neg(xyzw).x000\n"
		"mov r273.x___,r283.x000\n"
		"mov r434.x___,r271.y000\n"
		"mov r437.x___,r276.x000\n"
		"imax r435.x___,r276.x000,r437_neg(xyzw).x000\n"
		"mov r438.x___,r434.x000\n"
		"imax r436.x___,r434.x000,r438_neg(xyzw).x000\n"
		"udiv r439.x___,r435.x000,r436.x000\n"
		"inegate r440.x___,r439.x000\n"
		"ixor r441.x___,r276.x000,r434.x000\n"
		"and r441.x___,r441.x000,l5.x000\n"
		"cmov_logical r284.x___,r441.x000,r440.x000,r439.x000\n"
		"mov r273.__z_,r284.00x0\n"
		"mov r442.x___,r273.z000\n"
		"mov r443.x___,r271.y000\n"
		"imul r285.x___,r442.x000,r443.x000\n"
		"iadd r286.x___,r276.x000,r285_neg(xyzw).x000\n"
		"mov r273._y__,r286.0x00\n"
		"mov r269.xyzw,r273.xyzw\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"func 39\n"
		"mov r312.x___,l12.x000\n"
		"mov r312._y__,l15.0x00\n"
		"mov r309.xy__,r312.xy00\n"
		"mov r310.x___,l12.x000\n"
		"mov r311.x___,l12.x000\n"
		"mov r311.x___,r306.x000\n"
		"mov r446.x___,r311.x000\n"
		"imax r444.x___,r311.x000,r446_neg(xyzw).x000\n"
		"mov r447.x___,r308.x000\n"
		"imax r445.x___,r308.x000,r447_neg(xyzw).x000\n"
		"udiv r448.x___,r444.x000,r445.x000\n"
		"inegate r449.x___,r448.x000\n"
		"ixor r450.x___,r311.x000,r308.x000\n"
		"and r450.x___,r450.x000,l5.x000\n"
		"cmov_logical r313.x___,r450.x000,r449.x000,r448.x000\n"
		"mov r310.x___,r313.x000\n"
		"imul r314.x___,r310.x000,r308.x000\n"
		"iadd r315.x___,r311.x000,r314_neg(xyzw).x000\n"
		"itof r451.x___,r315.x000\n"
		"mov r316.x___,r451.x000\n"
		"add r317.x___,r316.x000,l16.x000\n"
		"mov r309.x___,r317.x000\n"
		"itof r452.x___,r310.x000\n"
		"mov r318.x___,r452.x000\n"
		"add r319.x___,r318.x000,l17.x000\n"
		"mov r309._y__,r319.0x00\n"
		"mov r305.xy__,r309.xy00\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"func 41\n"
		"mov r453.x___,r366.y000\n"
		"mov r373.x___,r453.x000\n"
		"mov r374.x___,r366.x000\n"
		"ige r375.x___,r373.x000,r360.x000\n"
		"iadd r376.x___,r360.x000,r361.x000\n"
		"ilt r377.x___,r373.x000,r376.x000\n"
		"and r378.x___,r375.x000,r377.x000\n"
		"if_logicalnz r378.x000\n"
		"iadd r384.x___,r361.x000,r373_neg(xyzw).x000\n"
		"iadd r385.x___,r384.x000,r360.x000\n"
		"iadd r386.x___,r385.x000,l14_neg(xyzw).x000\n"
		"ishr r387.x___,r386.x000,l14.x000\n"
		"mov r379.x___,r387.x000\n"
		"iadd r388.x___,r362.x000,r379.x000\n"
		"itof r454.x___,r388.x000\n"
		"mov r389.x___,r454.x000\n"
		"itof r455.x___,r374.x000\n"
		"mov r389._y__,r455.0x00\n"
		"mov r144.x___,r358.x000\n"
		"mov r145.xy__,r389.xy00\n"
		"call 20 \n"
		"mov r456.x___,r143.x000\n"
		"mov r390.x___,r456.x000\n"
		"mov r380.x___,r390.x000\n"
		"iadd r391.x___,r363.x000,r379_neg(xyzw).x000\n"
		"mov r457.xyzw,r391.xxxx\n"
		"mov r306.xyzw,r457.xyzw\n"
		"mov r307.xyzw,r369.xyzw\n"
		"mov r308.xyzw,r370.xyzw\n"
		"call 39 \n"
		"mov r458.xy__,r305.xy00\n"
		"mov r144.x___,r359.x000\n"
		"mov r145.xy__,r458.xy00\n"
		"call 20 \n"
		"mov r459.x___,r143.x000\n"
		"mov r392.x___,r459.x000\n"
		"mov r381.x___,r392.x000\n"
		"ieq r393.x___,r380.x000,r381.x000\n"
		"cmov_logical r394.x___,r393.x000,l14.x000,l12.x000\n"
		"mov r382.x___,r394.x000\n"
		"iadd r395.x___,r373.x000,r360_neg(xyzw).x000\n"
		"and r396.x___,r395.x000,l14.x000\n"
		"ieq r397.x___,r396.x000,l12.x000\n"
		"cmov_logical r398.x___,r397.x000,l14.x000,l12.x000\n"
		"mov r383.x___,r398.x000\n"
		"ieq r399.x___,r382.x000,l12.x000\n"
		"if_logicalnz r399.x000\n"
		"iadd r404.x___,r373.x000,r360_neg(xyzw).x000\n"
		"inot r405.x___,l14.x000\n"
		"and r406.x___,r404.x000,r405.x000\n"
		"iadd r407.x___,r406.x000,r360.x000\n"
		"mov r400.x___,r407.x000\n"
		"iadd r408.x___,r400.x000,l14.x000\n"
		"mov r401.x___,r408.x000\n"
		"itof r460.x___,r374.x000\n"
		"mov r409.x___,r460.x000\n"
		"itof r461.x___,r400.x000\n"
		"mov r409._y__,r461.0x00\n"
		"mov r144.x___,r364.x000\n"
		"mov r145.xy__,r409.xy00\n"
		"call 20 \n"
		"mov r462.x___,r143.x000\n"
		"mov r402.x___,r462.x000\n"
		"itof r463.x___,r374.x000\n"
		"mov r410.x___,r463.x000\n"
		"itof r464.x___,r401.x000\n"
		"mov r410._y__,r464.0x00\n"
		"mov r144.x___,r364.x000\n"
		"mov r145.xy__,r410.xy00\n"
		"call 20 \n"
		"mov r465.x___,r143.x000\n"
		"mov r403.x___,r465.x000\n"
		"ieq r411.x___,r383.x000,l12.x000\n"
		"if_logicalnz r411.x000\n"
		"imin r412.x___,r402.x000,r403.x000\n"
		"mov r365.x___,r412.x000\n"
		"else\n"
		"imax r413.x___,r402.x000,r403.x000\n"
		"mov r365.x___,r413.x000\n"
		"endif\n"
		"else\n"
		"itof r466.x___,r374.x000\n"
		"mov r414.x___,r466.x000\n"
		"itof r467.x___,r373.x000\n"
		"mov r414._y__,r467.0x00\n"
		"mov r144.x___,r364.x000\n"
		"mov r145.xy__,r414.xy00\n"
		"call 20 \n"
		"mov r468.x___,r143.x000\n"
		"mov r365.x___,r468.x000\n"
		"endif\n"
		"else\n"
		"itof r469.x___,r374.x000\n"
		"mov r415.x___,r469.x000\n"
		"itof r470.x___,r373.x000\n"
		"mov r415._y__,r470.0x00\n"
		"mov r144.x___,r364.x000\n"
		"mov r145.xy__,r415.xy00\n"
		"call 20 \n"
		"mov r471.x___,r143.x000\n"
		"mov r365.x___,r471.x000\n"
		"endif\n"
		"ret\n"
		"\n"
		"func 42\n"
		"mov r270.xy__,r417.xy00\n"
		"mov r271.xyzw,r356.xyzw\n"
		"mov r272.xyzw,r357.xyzw\n"
		"call 37 \n"
		"mov r472.xyzw,r269.xyzw\n"
		"mov r421.xyzw,r472.xyzw\n"
		"mov r420.xyzw,r421.xyzw\n"
		"mov r358.x___,l0.x000\n"
		"mov r359.x___,l1.x000\n"
		"mov r360.x___,r346.x000\n"
		"mov r361.x___,r348.x000\n"
		"mov r362.x___,r350.x000\n"
		"mov r363.x___,r352.x000\n"
		"mov r364.x___,l11.x000\n"
		"mov r366.xyzw,r420.xyzw\n"
		"mov r367.xyzw,r342.xyzw\n"
		"mov r368.xyzw,r343.xyzw\n"
		"mov r369.xyzw,r344.xyzw\n"
		"mov r370.xyzw,r345.xyzw\n"
		"mov r371.xyzw,r354.xyzw\n"
		"mov r372.xyzw,r355.xyzw\n"
		"call 41 \n"
		"mov r419.x___,r365.x000\n"
		"mov r422.x___,r419.x000\n"
		"mov r423.x___,r422.x000\n"
		"mov r423._y__,l12.0x00\n"
		"mov r423.__z_,l12.00x0\n"
		"mov r423.___w,l12.000x\n"
		"mov r418.xyzw,r423.xyzw\n"
		"mov r416.xyzw,r418.xyzw\n"
		"ret_dyn\n"
		"ret\n"
		"\n"
		"end\n"
		"";

	static const brook::KernelDesc __compnet_stage_2d_cal_desc = brook::KernelDesc()
		.technique( brook::Technique()
			.pass( brook::Pass( "__compnet_stage_2d_cal_desc_tech0_pass0 " )
				.image(__compnet_stage_2d_cal_desc_tech0_pass0)
				.constant(2, brook::CONST_USER_PARAM)
				.constant(3, brook::CONST_USER_PARAM)
				.constant(4, brook::CONST_USER_PARAM)
				.constant(5, brook::CONST_USER_PARAM)
				.input(0, 0, ACCESS_RANDOM)
				.input(1, 0, ACCESS_RANDOM)
				.input(6, 0, ACCESS_RANDOM)
				.output(7, 0, ACCESS_STREAM)
			)
		)
		.technique( brook::Technique()
			.addressTranslation()
			.pass( brook::Pass( "__compnet_stage_2d_cal_desc_tech1_pass0 " )
				.image(__compnet_stage_2d_cal_desc_tech1_pass0)
				.constant(0, brook::CONST_STREAMSHAPE)
				.constant(0, brook::CONST_BUFFERSHAPE)
				.constant(1, brook::CONST_STREAMSHAPE)
				.constant(1, brook::CONST_BUFFERSHAPE)
				.constant(2, brook::CONST_USER_PARAM)
				.constant(3, brook::CONST_USER_PARAM)
				.constant(4, brook::CONST_USER_PARAM)
				.constant(5, brook::CONST_USER_PARAM)
				.constant(6, brook::CONST_STREAMSHAPE)
				.constant(6, brook::CONST_BUFFERSHAPE)
				.constant(7, brook::CONST_STREAMSHAPE)
				.constant(7, brook::CONST_BUFFERSHAPE)
				.input(0, 0, ACCESS_RANDOM)
				.input(1, 0, ACCESS_RANDOM)
				.input(6, 0, ACCESS_RANDOM)
				.output(7, 0, ACCESS_STREAM)
			)
		);
	static const void* __compnet_stage_2d_cal = &__compnet_stage_2d_cal_desc;
}