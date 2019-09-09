#version 300 es

// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com

precision highp float;

// standard SimpleSceneGraph parameters
uniform vec3 SunDirTo;
uniform vec4 SunE0;

////////////////////////////////////////////////////////////////////////////////
// Standard SPHL library ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// U N I F O R M S /////////////////////////////////////////////////////////////

uniform samplerCube SPHL_LightProbe;
uniform vec4 SPHL_Coefs[120];
uniform int SPHL_NumDegrees;
uniform int SPHL_LightProbeMode;

// I N T E R F A C E ///////////////////////////////////////////////////////////

float calc_degree_0_mono(float theta, float phi);
float calc_degree_1_mono(float theta, float phi);
float calc_degree_2_mono(float theta, float phi);
float calc_degree_3_mono(float theta, float phi);
float calc_degree_4_mono(float theta, float phi);
float calc_degree_5_mono(float theta, float phi);
float calc_degree_6_mono(float theta, float phi);

vec3 calc_degree_0_rgb(float theta, float phi);
vec3 calc_degree_1_rgb(float theta, float phi);
vec3 calc_degree_2_rgb(float theta, float phi);
vec3 calc_degree_3_rgb(float theta, float phi);
vec3 calc_degree_4_rgb(float theta, float phi);
vec3 calc_degree_5_rgb(float theta, float phi);
vec3 calc_degree_6_rgb(float theta, float phi);

float SphlComputeMono(vec3 v);
vec3 SphlComputeRGB(vec3 v);

// C O N S T A N T S ///////////////////////////////////////////////////////////

const float SphlC_T1 = 0.2820947917738782;
const float SphlC_T2 = 1.0;
const float SphlC_T3 = .3454941494713354;
const float SphlC_T4 = 0.3454941494713354;
const float SphlC_T5 = 0.48860251190292;
const float SphlC_T6 = 1.0;
const float SphlC_T7 = 1.0;
const float SphlC_T8 = 1.0;
const float SphlC_T9 = 2.0;
const float SphlC_T10 = .3862742020231897;
const float SphlC_T11 = 0.3862742020231897;
const float SphlC_T12 = 0.7725484040463795;
const float SphlC_T13 = 0.7725484040463795;
const float SphlC_T14 = 0.6307831305050402;
const float SphlC_T15 = -3.0;
const float SphlC_T16 = 1.5;
const float SphlC_T17 = 1.0;
const float SphlC_T18 = 1.0;
const float SphlC_T19 = 1.0;
const float SphlC_T20 = 1.0;
const float SphlC_T21 = -2.5;
const float SphlC_T22 = 1.25;
const float SphlC_T23 = 1.0;
const float SphlC_T24 = 2.0;
const float SphlC_T25 = 3.0;
const float SphlC_T26 = -0.4172238236327842;
const float SphlC_T27 = 0.4172238236327842;
const float SphlC_T28 = 1.021985476433283;
const float SphlC_T29 = 1.021985476433283;
const float SphlC_T30 = 1.292720736456603;
const float SphlC_T31 = 1.292720736456603;
const float SphlC_T32 = 0.7463526651802309;
const float SphlC_T33 = -6.0;
const float SphlC_T34 = 2.5;
const float SphlC_T35 = 7.5;
const float SphlC_T36 = 1.0;
const float SphlC_T37 = 1.0;
const float SphlC_T38 = 1.0;
const float SphlC_T39 = 1.0;
const float SphlC_T40 = -4.5;
const float SphlC_T41 = 1.75;
const float SphlC_T42 = 5.25;
const float SphlC_T43 = 1.0;
const float SphlC_T44 = 2.0;
const float SphlC_T45 = -2.333333333333333;
const float SphlC_T46 = 1.166666666666667;
const float SphlC_T47 = 1.0;
const float SphlC_T48 = 3.0;
const float SphlC_T49 = 4.0;
const float SphlC_T50 = .4425326924449827;
const float SphlC_T51 = 0.4425326924449827;
const float SphlC_T52 = 1.251671470898353;
const float SphlC_T53 = 1.251671470898353;
const float SphlC_T54 = 2.007139630671868;
const float SphlC_T55 = 2.007139630671868;
const float SphlC_T56 = 1.89234939151512;
const float SphlC_T57 = 1.89234939151512;
const float SphlC_T58 = 0.8462843753216346;
const float SphlC_T59 = -10.0;
const float SphlC_T60 = 4.375;
const float SphlC_T61 = 17.5;
const float SphlC_T62 = 22.5;
const float SphlC_T63 = 1.0;
const float SphlC_T64 = 1.0;
const float SphlC_T65 = 1.0;
const float SphlC_T66 = 1.0;
const float SphlC_T67 = -7.0;
const float SphlC_T68 = 2.625;
const float SphlC_T69 = 10.5;
const float SphlC_T70 = 14.0;
const float SphlC_T71 = 1.0;
const float SphlC_T72 = 2.0;
const float SphlC_T73 = -4.0;
const float SphlC_T74 = 1.5;
const float SphlC_T75 = 4.5;
const float SphlC_T76 = 1.0;
const float SphlC_T77 = 3.0;
const float SphlC_T78 = -2.25;
const float SphlC_T79 = 1.125;
const float SphlC_T80 = 1.0;
const float SphlC_T81 = 4.0;
const float SphlC_T82 = 5.0;
const float SphlC_T83 = .4641322034408583;
const float SphlC_T84 = 0.4641322034408583;
const float SphlC_T85 = 1.467714898305751;
const float SphlC_T86 = 1.467714898305751;
const float SphlC_T87 = 2.767549753174722;
const float SphlC_T88 = 2.767549753174722;
const float SphlC_T89 = 3.389542366521799;
const float SphlC_T90 = 3.389542366521799;
const float SphlC_T91 = 2.562253188609721;
const float SphlC_T92 = 2.562253188609721;
const float SphlC_T93 = 0.9356025796273889;
const float SphlC_T94 = -15.0;
const float SphlC_T95 = 7.875;
const float SphlC_T96 = 39.375;
const float SphlC_T97 = 70.0;
const float SphlC_T98 = 52.5;
const float SphlC_T99 = 1.0;
const float SphlC_T100 = 1.0;
const float SphlC_T101 = 1.0;
const float SphlC_T102 = 1.0;
const float SphlC_T103 = -10.0;
const float SphlC_T104 = 4.125;
const float SphlC_T105 = 20.625;
const float SphlC_T106 = 37.5;
const float SphlC_T107 = 30.0;
const float SphlC_T108 = 1.0;
const float SphlC_T109 = 2.0;
const float SphlC_T110 = -6.0;
const float SphlC_T111 = 2.0625;
const float SphlC_T112 = 8.25;
const float SphlC_T113 = 11.25;
const float SphlC_T114 = 1.0;
const float SphlC_T115 = 3.0;
const float SphlC_T116 = -3.75;
const float SphlC_T117 = 1.375;
const float SphlC_T118 = 4.125;
const float SphlC_T119 = 1.0;
const float SphlC_T120 = 4.0;
const float SphlC_T121 = -2.2;
const float SphlC_T122 = 1.1;
const float SphlC_T123 = 1.0;
const float SphlC_T124 = 5.0;
const float SphlC_T125 = 6.0;
const float SphlC_T126 = 0.4830841135800663;
const float SphlC_T127 = 0.4830841135800663;
const float SphlC_T128 = 1.673452458100098;
const float SphlC_T129 = 1.673452458100098;
const float SphlC_T130 = 3.567812628539981;
const float SphlC_T131 = 3.567812628539981;
const float SphlC_T132 = 5.211123886941726;
const float SphlC_T133 = 5.211123886941726;
const float SphlC_T134 = 5.211123886941726;
const float SphlC_T135 = 5.211123886941726;
const float SphlC_T136 = 3.295804130409127;
const float SphlC_T137 = 3.295804130409127;
const float SphlC_T138 = 1.017107236282055;
const float SphlC_T139 = -21.0;
const float SphlC_T140 = 14.4375;
const float SphlC_T141 = 86.625;
const float SphlC_T142 = 196.875;
const float SphlC_T143 = 210.0;
const float SphlC_T144 = 105.0;
const float SphlC_T145 = 1.0;


// I M P L E M E N T A T I O N /////////////////////////////////////////////////


float calc_degree_0_mono(float theta, float phi) {
	return SphlC_T1*SPHL_Coefs[0].a;
}


float calc_degree_1_mono(float theta, float phi) {
	float t1 = cos(SphlC_T2*phi);
	float t2 = sin(theta);
	return SphlC_T3*t1*SPHL_Coefs[3].a * t2 - SphlC_T4*t1*SPHL_Coefs[1].a * t2 + SphlC_T5*SPHL_Coefs[2].a * cos(theta);
}


float calc_degree_2_mono(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T6 - SphlC_T7*t1;
	float t3 = cos(SphlC_T8*phi);
	float t4 = sin(theta);
	float t5 = cos(SphlC_T9*phi);
	float t6 = pow(t4, 2.0);
	return SphlC_T10*t5*SPHL_Coefs[8].a * t6 + SphlC_T11*t5*SPHL_Coefs[4].a * t6 + SphlC_T12*t3*SPHL_Coefs[7].a * t1*t4 - SphlC_T13*t3*SPHL_Coefs[5].a * t1*t4 + SphlC_T14*SPHL_Coefs[6].a * (SphlC_T15*t2 + SphlC_T16*t2*t2 + SphlC_T17);
}


float calc_degree_3_mono(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T18 - SphlC_T19*t1;
	float t3 = pow(t2, 2.0);
	float t4 = cos(SphlC_T20*phi);
	float t5 = SphlC_T21*t2 + SphlC_T22*t3 + SphlC_T23;
	float t6 = sin(theta);
	float t7 = cos(SphlC_T24*phi);
	float t8 = pow(t6, 2.0);
	float t9 = cos(SphlC_T25*phi);
	float t10 = pow(t6, 3.0);
	return SphlC_T26*t9*SPHL_Coefs[9].a * t10 + SphlC_T27*t9*SPHL_Coefs[15].a * t10 + SphlC_T28*t7*SPHL_Coefs[14].a * t1*t8 + SphlC_T29*t7*SPHL_Coefs[10].a * t1*t8 +
		SphlC_T30*t4*SPHL_Coefs[13].a * t5*t6 - SphlC_T31*t4*SPHL_Coefs[11].a * t5*t6 +
		SphlC_T32*SPHL_Coefs[12].a * (SphlC_T33*t2 - SphlC_T34*pow(t2, 3.0) + SphlC_T35*t3 + SphlC_T36);
}


float calc_degree_4_mono(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T37 - SphlC_T38*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = cos(SphlC_T39*phi);
	float t6 = SphlC_T40*t2 - SphlC_T41*t4 + SphlC_T42*t3 + SphlC_T43;
	float t7 = sin(theta);
	float t8 = cos(SphlC_T44*phi);
	float t9 = SphlC_T45*t2 + SphlC_T46*t3 + SphlC_T47;
	float t10 = pow(t7, 2.0);
	float t11 = cos(SphlC_T48*phi);
	float t12 = pow(t7, 3.0);
	float t13 = cos(SphlC_T49*phi);
	float t14 = pow(t7, 4.0);
	return SphlC_T50*t13*SPHL_Coefs[24].a * t14 + SphlC_T51*t13*SPHL_Coefs[16].a * t14 + SphlC_T52*t11*SPHL_Coefs[23].a * t1*t12 -
		SphlC_T53*t11*SPHL_Coefs[17].a * t1*t12 + SphlC_T54*t8*SPHL_Coefs[22].a * t9*t10 + SphlC_T55*t8*SPHL_Coefs[18].a * t9*t10 +
		SphlC_T56*t5*SPHL_Coefs[21].a * t6*t7 - SphlC_T57*t5*SPHL_Coefs[19].a * t6*t7 +
		SphlC_T58*SPHL_Coefs[20].a * (SphlC_T59*t2 + SphlC_T60*pow(t2, 4.0) - SphlC_T61*t4 + SphlC_T62*t3 + SphlC_T63);
}


float calc_degree_5_mono(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T64 - SphlC_T65*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = pow(t2, 4.0);
	float t6 = cos(SphlC_T66*phi);
	float t7 = SphlC_T67*t2 + SphlC_T68*t5 - SphlC_T69*t4 + SphlC_T70*t3 + SphlC_T71;
	float t8 = sin(theta);
	float t9 = cos(SphlC_T72*phi);
	float t10 = SphlC_T73*t2 - SphlC_T74*t4 + SphlC_T75*t3 + SphlC_T76;
	float t11 = pow(t8, 2.0);
	float t12 = cos(SphlC_T77*phi);
	float t13 = SphlC_T78*t2 + SphlC_T79*t3 + SphlC_T80;
	float t14 = pow(t8, 3.0);
	float t15 = cos(SphlC_T81*phi);
	float t16 = pow(t8, 4.0);
	float t17 = cos(SphlC_T82*phi);
	float t18 = pow(t8, 5.0);
	return SphlC_T83*t17*SPHL_Coefs[35].a * t18 - SphlC_T84*t17*SPHL_Coefs[25].a * t18 + SphlC_T85*t15*SPHL_Coefs[34].a * t1*t16 +
		SphlC_T86*t15*SPHL_Coefs[26].a * t1*t16 + SphlC_T87*t12*SPHL_Coefs[33].a * t13*t14 - SphlC_T88*t12*SPHL_Coefs[27].a * t13*t14 +
		SphlC_T89*t9*SPHL_Coefs[32].a * t10*t11 + SphlC_T90*t9*SPHL_Coefs[28].a * t10*t11 + SphlC_T91*t6*SPHL_Coefs[31].a * t7*t8 -
		SphlC_T92*t6*SPHL_Coefs[29].a * t7*t8 +
		SphlC_T93*SPHL_Coefs[30].a * (SphlC_T94*t2 - SphlC_T95*pow(t2, 5.0) + SphlC_T96*t5 - SphlC_T97*t4 + SphlC_T98*t3 + SphlC_T99);
}


float calc_degree_6_mono(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T100 - SphlC_T101*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = pow(t2, 4.0);
	float t6 = pow(t2, 5.0);
	float t7 = cos(SphlC_T102*phi);
	float t8 = SphlC_T103*t2 - SphlC_T104*t6 + SphlC_T105*t5 - SphlC_T106*t4 + SphlC_T107*t3 + SphlC_T108;
	float t9 = sin(theta);
	float t10 = cos(SphlC_T109*phi);
	float t11 = SphlC_T110*t2 + SphlC_T111*t5 - SphlC_T112*t4 + SphlC_T113*t3 + SphlC_T114;
	float t12 = pow(t9, 2.0);
	float t13 = cos(SphlC_T115*phi);
	float t14 = SphlC_T116*t2 - SphlC_T117*t4 + SphlC_T118*t3 + SphlC_T119;
	float t15 = pow(t9, 3.0);
	float t16 = cos(SphlC_T120*phi);
	float t17 = SphlC_T121*t2 + SphlC_T122*t3 + SphlC_T123;
	float t18 = pow(t9, 4.0);
	float t19 = cos(SphlC_T124*phi);
	float t20 = pow(t9, 5.0);
	float t21 = cos(SphlC_T125*phi);
	float t22 = pow(t9, 6.0);
	return SphlC_T126*t21*SPHL_Coefs[48].a * t22 + SphlC_T127*t21*SPHL_Coefs[36].a * t22 + SphlC_T128*t19*SPHL_Coefs[47].a * t1*t20 -
		SphlC_T129*t19*SPHL_Coefs[37].a * t1*t20 + SphlC_T130*t16*SPHL_Coefs[46].a * t17*t18 + SphlC_T131*t16*SPHL_Coefs[38].a * t17*t18 +
		SphlC_T132*t13*SPHL_Coefs[45].a * t14*t15 - SphlC_T133*t13*SPHL_Coefs[39].a * t14*t15 + SphlC_T134*t10*SPHL_Coefs[44].a * t11*t12 +
		SphlC_T135*t10*SPHL_Coefs[40].a * t11*t12 + SphlC_T136*t7*SPHL_Coefs[43].a * t8*t9 - SphlC_T137*t7*SPHL_Coefs[41].a * t8*t9 +
		SphlC_T138*SPHL_Coefs[42].a * (SphlC_T139*t2 + SphlC_T140*pow(t2, 6.0) - SphlC_T141*t6 + SphlC_T142*t5 - SphlC_T143*t4 + SphlC_T144*t3 + SphlC_T145);
}


vec3 calc_degree_0_rgb(float theta, float phi) {
	return SphlC_T1*SPHL_Coefs[0].rgb;
}


vec3 calc_degree_1_rgb(float theta, float phi) {
	float t1 = cos(SphlC_T2*phi);
	float t2 = sin(theta);
	return SphlC_T3*t1*SPHL_Coefs[3].rgb * t2 - SphlC_T4*t1*SPHL_Coefs[1].rgb * t2 + SphlC_T5*SPHL_Coefs[2].rgb * cos(theta);
}


vec3 calc_degree_2_rgb(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T6 - SphlC_T7*t1;
	float t3 = cos(SphlC_T8*phi);
	float t4 = sin(theta);
	float t5 = cos(SphlC_T9*phi);
	float t6 = pow(t4, 2.0);
	return SphlC_T10*t5*SPHL_Coefs[8].rgb * t6 + SphlC_T11*t5*SPHL_Coefs[4].rgb * t6 + SphlC_T12*t3*SPHL_Coefs[7].rgb * t1*t4 - SphlC_T13*t3*SPHL_Coefs[5].rgb * t1*t4 + SphlC_T14*SPHL_Coefs[6].rgb * (SphlC_T15*t2 + SphlC_T16*t2*t2 + SphlC_T17);
}


vec3 calc_degree_3_rgb(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T18 - SphlC_T19*t1;
	float t3 = pow(t2, 2.0);
	float t4 = cos(SphlC_T20*phi);
	float t5 = SphlC_T21*t2 + SphlC_T22*t3 + SphlC_T23;
	float t6 = sin(theta);
	float t7 = cos(SphlC_T24*phi);
	float t8 = pow(t6, 2.0);
	float t9 = cos(SphlC_T25*phi);
	float t10 = pow(t6, 3.0);
	return SphlC_T26*t9*SPHL_Coefs[9].rgb * t10 + SphlC_T27*t9*SPHL_Coefs[15].rgb * t10 + SphlC_T28*t7*SPHL_Coefs[14].rgb * t1*t8 + SphlC_T29*t7*SPHL_Coefs[10].rgb * t1*t8 +
		SphlC_T30*t4*SPHL_Coefs[13].rgb * t5*t6 - SphlC_T31*t4*SPHL_Coefs[11].rgb * t5*t6 +
		SphlC_T32*SPHL_Coefs[12].rgb * (SphlC_T33*t2 - SphlC_T34*pow(t2, 3.0) + SphlC_T35*t3 + SphlC_T36);
}


vec3 calc_degree_4_rgb(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T37 - SphlC_T38*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = cos(SphlC_T39*phi);
	float t6 = SphlC_T40*t2 - SphlC_T41*t4 + SphlC_T42*t3 + SphlC_T43;
	float t7 = sin(theta);
	float t8 = cos(SphlC_T44*phi);
	float t9 = SphlC_T45*t2 + SphlC_T46*t3 + SphlC_T47;
	float t10 = pow(t7, 2.0);
	float t11 = cos(SphlC_T48*phi);
	float t12 = pow(t7, 3.0);
	float t13 = cos(SphlC_T49*phi);
	float t14 = pow(t7, 4.0);
	return SphlC_T50*t13*SPHL_Coefs[24].rgb * t14 + SphlC_T51*t13*SPHL_Coefs[16].rgb * t14 + SphlC_T52*t11*SPHL_Coefs[23].rgb * t1*t12 -
		SphlC_T53*t11*SPHL_Coefs[17].rgb * t1*t12 + SphlC_T54*t8*SPHL_Coefs[22].rgb * t9*t10 + SphlC_T55*t8*SPHL_Coefs[18].rgb * t9*t10 +
		SphlC_T56*t5*SPHL_Coefs[21].rgb * t6*t7 - SphlC_T57*t5*SPHL_Coefs[19].rgb * t6*t7 +
		SphlC_T58*SPHL_Coefs[20].rgb * (SphlC_T59*t2 + SphlC_T60*pow(t2, 4.0) - SphlC_T61*t4 + SphlC_T62*t3 + SphlC_T63);
}


vec3 calc_degree_5_rgb(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T64 - SphlC_T65*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = pow(t2, 4.0);
	float t6 = cos(SphlC_T66*phi);
	float t7 = SphlC_T67*t2 + SphlC_T68*t5 - SphlC_T69*t4 + SphlC_T70*t3 + SphlC_T71;
	float t8 = sin(theta);
	float t9 = cos(SphlC_T72*phi);
	float t10 = SphlC_T73*t2 - SphlC_T74*t4 + SphlC_T75*t3 + SphlC_T76;
	float t11 = pow(t8, 2.0);
	float t12 = cos(SphlC_T77*phi);
	float t13 = SphlC_T78*t2 + SphlC_T79*t3 + SphlC_T80;
	float t14 = pow(t8, 3.0);
	float t15 = cos(SphlC_T81*phi);
	float t16 = pow(t8, 4.0);
	float t17 = cos(SphlC_T82*phi);
	float t18 = pow(t8, 5.0);
	return SphlC_T83*t17*SPHL_Coefs[35].rgb * t18 - SphlC_T84*t17*SPHL_Coefs[25].rgb * t18 + SphlC_T85*t15*SPHL_Coefs[34].rgb * t1*t16 +
		SphlC_T86*t15*SPHL_Coefs[26].rgb * t1*t16 + SphlC_T87*t12*SPHL_Coefs[33].rgb * t13*t14 - SphlC_T88*t12*SPHL_Coefs[27].rgb * t13*t14 +
		SphlC_T89*t9*SPHL_Coefs[32].rgb * t10*t11 + SphlC_T90*t9*SPHL_Coefs[28].rgb * t10*t11 + SphlC_T91*t6*SPHL_Coefs[31].rgb * t7*t8 -
		SphlC_T92*t6*SPHL_Coefs[29].rgb * t7*t8 +
		SphlC_T93*SPHL_Coefs[30].rgb * (SphlC_T94*t2 - SphlC_T95*pow(t2, 5.0) + SphlC_T96*t5 - SphlC_T97*t4 + SphlC_T98*t3 + SphlC_T99);
}


vec3 calc_degree_6_rgb(float theta, float phi) {
	float t1 = cos(theta);
	float t2 = SphlC_T100 - SphlC_T101*t1;
	float t3 = pow(t2, 2.0);
	float t4 = pow(t2, 3.0);
	float t5 = pow(t2, 4.0);
	float t6 = pow(t2, 5.0);
	float t7 = cos(SphlC_T102*phi);
	float t8 = SphlC_T103*t2 - SphlC_T104*t6 + SphlC_T105*t5 - SphlC_T106*t4 + SphlC_T107*t3 + SphlC_T108;
	float t9 = sin(theta);
	float t10 = cos(SphlC_T109*phi);
	float t11 = SphlC_T110*t2 + SphlC_T111*t5 - SphlC_T112*t4 + SphlC_T113*t3 + SphlC_T114;
	float t12 = pow(t9, 2.0);
	float t13 = cos(SphlC_T115*phi);
	float t14 = SphlC_T116*t2 - SphlC_T117*t4 + SphlC_T118*t3 + SphlC_T119;
	float t15 = pow(t9, 3.0);
	float t16 = cos(SphlC_T120*phi);
	float t17 = SphlC_T121*t2 + SphlC_T122*t3 + SphlC_T123;
	float t18 = pow(t9, 4.0);
	float t19 = cos(SphlC_T124*phi);
	float t20 = pow(t9, 5.0);
	float t21 = cos(SphlC_T125*phi);
	float t22 = pow(t9, 6.0);
	return SphlC_T126*t21*SPHL_Coefs[48].rgb * t22 + SphlC_T127*t21*SPHL_Coefs[36].rgb * t22 + SphlC_T128*t19*SPHL_Coefs[47].rgb * t1*t20 -
		SphlC_T129*t19*SPHL_Coefs[37].rgb * t1*t20 + SphlC_T130*t16*SPHL_Coefs[46].rgb * t17*t18 + SphlC_T131*t16*SPHL_Coefs[38].rgb * t17*t18 +
		SphlC_T132*t13*SPHL_Coefs[45].rgb * t14*t15 - SphlC_T133*t13*SPHL_Coefs[39].rgb * t14*t15 + SphlC_T134*t10*SPHL_Coefs[44].rgb * t11*t12 +
		SphlC_T135*t10*SPHL_Coefs[40].rgb * t11*t12 + SphlC_T136*t7*SPHL_Coefs[43].rgb * t8*t9 - SphlC_T137*t7*SPHL_Coefs[41].rgb * t8*t9 +
		SphlC_T138*SPHL_Coefs[42].rgb * (SphlC_T139*t2 + SphlC_T140*pow(t2, 6.0) - SphlC_T141*t6 + SphlC_T142*t5 - SphlC_T143*t4 + SphlC_T144*t3 + SphlC_T145);
}


float SphlComputeMono(vec3 v)
{
	float accum = 0.0;
	float theta = atan(v.y, v.x);
	float phi = acos(v.z);

	// switch(SPHL_NumDegrees)
	// {
	// 	case 10:
	// 	case 9:
	// 	case 8:
	// 	case 7:
	// 	case 6:	accum += calc_degree_6_mono(theta, phi);
	// 	case 5: accum += calc_degree_5_mono(theta, phi);
	// 	case 4: accum += calc_degree_4_mono(theta, phi);
	// 	case 3: accum += calc_degree_3_mono(theta, phi);
	// 	case 2: accum += calc_degree_2_mono(theta, phi);
	// 	case 1: accum += calc_degree_1_mono(theta, phi);
	// 	case 0: accum += calc_degree_0_mono(theta, phi);
	// }

	if (SPHL_NumDegrees >= 6)
	{
		accum += calc_degree_6_mono(theta, phi);
		accum += calc_degree_5_mono(theta, phi);
		accum += calc_degree_4_mono(theta, phi);
		accum += calc_degree_3_mono(theta, phi);
		accum += calc_degree_2_mono(theta, phi);
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);		
	}
	else if (SPHL_NumDegrees >= 5)
	{
		accum += calc_degree_5_mono(theta, phi);
		accum += calc_degree_4_mono(theta, phi);
		accum += calc_degree_3_mono(theta, phi);
		accum += calc_degree_2_mono(theta, phi);
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 4)
	{
		accum += calc_degree_5_mono(theta, phi);
		accum += calc_degree_4_mono(theta, phi);
		accum += calc_degree_3_mono(theta, phi);
		accum += calc_degree_2_mono(theta, phi);
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 3)
	{
		accum += calc_degree_3_mono(theta, phi);
		accum += calc_degree_2_mono(theta, phi);
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 2)
	{
		accum += calc_degree_2_mono(theta, phi);
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 1)
	{
		accum += calc_degree_1_mono(theta, phi);
		accum += calc_degree_0_mono(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 0)
	{
		accum += calc_degree_0_mono(theta, phi);	
	}
	return accum;
}


vec3 SphlComputeRGB(vec3 v)
{
	vec3 accum = vec3(0.0, 0.0, 0.0);
	float theta = atan(v.y, v.x);
	float phi = acos(v.z);

	// switch(SPHL_NumDegrees)
	// {
	// 	case 10:
	// 	case 9:
	// 	case 8:
	// 	case 7:
	// 	case 6:	accum += calc_degree_6_rgb(theta, phi);
	// 	case 5: accum += calc_degree_5_rgb(theta, phi);
	// 	case 4: accum += calc_degree_4_rgb(theta, phi);
	// 	case 3: accum += calc_degree_3_rgb(theta, phi);
	// 	case 2: accum += calc_degree_2_rgb(theta, phi);
	// 	case 1: accum += calc_degree_1_rgb(theta, phi);
	// 	case 0: accum += calc_degree_0_rgb(theta, phi);
	// }

	if (SPHL_NumDegrees >= 6)
	{
		accum += calc_degree_6_rgb(theta, phi);
		accum += calc_degree_5_rgb(theta, phi);
		accum += calc_degree_4_rgb(theta, phi);
		accum += calc_degree_3_rgb(theta, phi);
		accum += calc_degree_2_rgb(theta, phi);
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);		
	}
	else if (SPHL_NumDegrees >= 5)
	{
		accum += calc_degree_5_rgb(theta, phi);
		accum += calc_degree_4_rgb(theta, phi);
		accum += calc_degree_3_rgb(theta, phi);
		accum += calc_degree_2_rgb(theta, phi);
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 4)
	{
		accum += calc_degree_5_rgb(theta, phi);
		accum += calc_degree_4_rgb(theta, phi);
		accum += calc_degree_3_rgb(theta, phi);
		accum += calc_degree_2_rgb(theta, phi);
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 3)
	{
		accum += calc_degree_3_rgb(theta, phi);
		accum += calc_degree_2_rgb(theta, phi);
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 2)
	{
		accum += calc_degree_2_rgb(theta, phi);
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 1)
	{
		accum += calc_degree_1_rgb(theta, phi);
		accum += calc_degree_0_rgb(theta, phi);	
	}
	else if (SPHL_NumDegrees >= 0)
	{
		accum += calc_degree_0_rgb(theta, phi);	
	}
	return accum;
}


////////////////////////////////////////////////////////////////////////////////
// End Standard SPHL Library ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// Input from Vertex Shader
in vec3 vNormal;
in vec4 vColor;
in vec3 vTexCoord;
in vec3 vViewVector;
in vec4 vPosition;

out vec4 FragColor;

void main()
{
	vec4 dP1 = dFdx(vPosition);
	vec4 dP2 = dFdy(vPosition);
	vec3 fragN = normalize(cross(dP1.xyz, dP2.xyz));	
	vec4 outputColor = vec4(0.0);
	vec3 N = normalize(vNormal);
	vec3 V = normalize(vViewVector);
	float NdotL = dot(N, SunDirTo);
	float NdotH = max(0.0, dot(normalize(SunDirTo + V), N));
	float cos_theta = 0.5 * (1.0 + NdotL);
	vec4 tint = vec4(0.5 * N + 0.5, 0.0);
	// if (NdotL < 0.0) {
	// 	tint = vec4(0.0, 0.0, -NdotL, 0.0);
	// }

	//if (NdotL > 0.0)
	{
		vec4 color_d = vec4(vColor.rgb, 0.20);// + (1.0 - cos_theta);
		vec4 color_s = vec4(SunE0.rgb, 0.0);
		outputColor += color_d * max(0.0, cos_theta);
		outputColor += color_s * pow(NdotH, 250.0);
		outputColor += tint * 0.15;
		//outputColor += vec4(SunDirTo.rgb, 0.0);		
	}		

	if (SPHL_LightProbeMode != 0)
	{
		outputColor = texture(SPHL_LightProbe, vTexCoord);
	}
	else
	{
		//outputColor = texture(SPHL_LightProbe, vTexCoord);		
	}

	FragColor = outputColor;
}
