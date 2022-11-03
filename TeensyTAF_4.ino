//BIRDTAF variant running on Teensy 4
//Hamish Mehaffey,  Apr 2020.
//as always, a work in progress. No versioning, not particularly graceful, but feel free to fix it.
//
// William.Mehaffey@ucsf.edu
// Distributed as-is. No warranty asked or received.
//
//

#
// ARM library.....
#define ARM_MATH_CM7
#include <arm_math.h>

//#include "AudioStream.h"  //comment this out to return to the original version
#include "utility/sqrt_integer.h"

//These are the includes from the Teensy Audio Library
#include <Audio.h>      //Teensy Audio Library
#include <Wire.h>
#include <SPI.h>

// Create audio objects
AudioControlSGTL5000     audioShield;   //controller for the Teensy Audio Board
AudioFilterStateVariable FilterLP;
AudioFilterStateVariable FilterHP;
AudioFilterStateVariable FilterHPOut;
AudioInputI2S            i2s_in;        //Digital audio *from* the Teensy Audio Board ADC.  Sends Int16.  Stereo.
AudioRecordQueue         QueueIn;
AudioOutputI2S           i2s_out;  // USB fucked at low AUDIO_BLOCK_SAMPLES, so stuck with i2sout for now. 
AudioPlayQueue           QueueOutSound;
AudioPlayMemory          NoiseOut;
//AudioPlaySdWav           SDAudioOut;
AudioSynthNoiseWhite     WNSynth;       

// Make all of the audio connections
AudioConnection         patchCord1(i2s_in, 0, FilterHP,0);
AudioConnection         patchCord2(FilterHP, 2, FilterLP, 0);
AudioConnection         patchCord3(FilterLP, 0, QueueIn, 0);
AudioConnection         patchCord4(QueueOutSound, 0, i2s_out, 0); // left
AudioConnection         patchCord8(WNSynth, 0, FilterHPOut, 0);
AudioConnection         patchCord9(FilterHPOut, 2, i2s_out, 1);


// ARM setup
arm_status status;
extern const int16_t AudioWindowHanning1024[];

////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION
// Values controlling the acquisition, template matching, amplitude thresholds, etc.
////////////////////////////////////////////////////////////////////////////////

///////////// LEDs. Probably not necessary.
const int POWER_LED_PIN = 13;          // Output pin for power LED (pin 13 = onboard LED).

///////////////
// Analog Input Settings for Sound. Note that this does not effect AUDIO_MIRROR_OUT (on PCB), which is hardware w/ 0 lag.
//////////////
int16_t audio[AUDIO_BLOCK_SAMPLES];

//int16_t WN[2205]={4808,16399,-20200,7710,2850,-11695,-3878,3063,32000,24765,-12072,27139,6486,-564,6391,-1833,-1111,13321,12600,12673,6004,-10799,6413,14578,4371,9252,6500,-2714,2627,-7041,7944,-10258,-9559,-7239,-26330,12862,2908,-6751,12253,-15306,-915,-2160,2854,2797,-7735,-269,-1475,5613,9776,9919,-7724,691,-10858,-9958,-62,13705,-6883,3321,-2018,9992,-9740,291,4941,9842,13809,768,-13339,-6639,-9494,21019,-5506,6689,-1721,7946,-6840,-12540,-12720,4365,-1587,-1754,12692,2607,1768,14198,-7194,6229,7467,-2180,1928,-10426,-10266,937,6458,23120,-5964,1675,-738,-17287,-3926,-16050,7515,-7942,895,-4870,2714,-5369,4381,6611,15308,-1736,-19123,-7509,12113,-9588,8593,1109,12847,-17536,-1768,-10802,26005,7379,12331,-9463,-4191,-2437,9822,-2485,6273,-18349,-3165,-7365,-14103,4542,2521,299,-11927,10082,3131,-2675,204,-2343,-15652,-2555,-7435,-8757,-10342,-4772,-17909,8622,4650,-180,-311,-7138,9109,-1192,-6390,12084,-2011,-5268,-2627,-7583,-10017,22588,14804,2750,-11242,-7740,-1579,7077,-11912,-20835,-12959,2982,3499,4039,-1166,1642,-4259,7708,-12178,4069,-7590,-2995,4943,9292,-9995,11273,5903,-607,-1746,-1946,-2711,206,458,7387,13655,4175,-1876,5590,1638,-9209,8488,2745,1208,4607,2337,-8420,-1452,-1307,-4758,15042,-7832,-4327,-6368,-10501,-1720,-2451,13682,-2227,-9517,14338,11041,-2054,-13469,-3977,-1395,2468,-2336,3965,3504,-11185,-8478,-6628,-4542,-2867,111,-27089,-4087,11110,-9540,8349,3132,-260,1631,-13996,-756,14343,879,369,-6566,-276,2077,3812,-3334,-2115,18096,-20196,19936,3018,8943,-14882,-5277,-2487,3780,-14936,4217,-10846,591,5833,2924,9681,8996,-5821,2298,-8446,-11821,8270,0,-492,8147,5317,3131,11180,8314,2144,-6174,-5827,10660,-14414,-219,-17428,9125,7705,10,-634,-22234,5197,-19606,-20741,714,-8482,3679,6053,7670,-6181,4018,899,7387,4794,8029,-1180,-1317,9012,-18991,-4513,-11363,-3422,5800,7384,-9077,-4213,1225,-2611,2699,3576,-8317,-1582,-19067,10242,-5626,-10766,-2271,-12776,-187,-5014,19474,10180,-22329,3946,-12503,-2281,1470,6686,-2442,14096,-4301,2928,5944,761,7877,2890,-7013,-16145,16620,-5407,924,5036,1015,-8091,-4183,-1117,13225,-7698,7016,2759,-2092,-9453,-2541,-776,-13141,1718,-7354,-843,3006,-8090,-2578,3130,-16418,9264,21680,8579,-2824,3832,-9265,16792,8412,7040,-7833,2861,-4993,-2785,-5098,-9173,-8127,-1878,-15193,5433,-1054,6252,2411,4420,-13263,-9124,-3998,980,10093,-2594,11281,4251,10499,1135,-5874,-13248,1390,7319,-2617,-4837,-2761,-9807,-4409,-1617,409,-571,5466,977,16221,2790,16136,-6467,4708,-2328,5366,5311,-19549,-11868,-12887,3593,13147,-2923,7264,4878,-9405,3554,-6724,13559,-292,14630,-3802,5271,-562,-18082,-8783,5477,-491,-10005,-5602,2231,-8881,8718,-5730,16175,-9657,1781,-13602,-6472,-5306,3588,8425,2687,-3337,7292,7144,1074,5108,3691,-8826,6792,-5878,-5401,1582,-2750,-1179,5324,9361,-1771,2930,-2132,2053,3934,-5517,2457,5375,825,15469,-5443,-6592,-15649,8142,7753,-715,8034,1642,2600,1010,3934,909,24925,-10433,-16583,-10201,-9778,-3878,-1507,-1955,4840,3481,6717,15902,10937,-11476,-20827,8065,-16416,596,317,19916,-619,-4537,2108,2198,626,-5443,-10934,2830,-12009,-9231,11904,-3747,-1255,8046,-2684,9205,-3086,9057,5627,-1905,-7742,-9329,-2416,-3919,-3655,8795,-2663,10227,-4755,8697,-4671,1579,8680,-3702,-3920,17915,8504,-3864,5803,-3221,6312,12661,-14349,9200,13037,424,15615,1389,-11064,-19616,-2982,6380,2838,3698,-5161,1287,-14654,-6798,-7323,4647,-127,-10334,-86,-6169,-5962,7727,1014,3562,7904,1611,4926,6107,10468,4255,12628,202,-429,15214,-4559,-26,8225,1339,12563,9247,2607,-6955,5067,-12365,2186,7229,1905,7866,18232,8262,2386,5738,3804,-11757,-3724,10951,-390,5208,-9001,576,5368,-12176,3108,-1627,-8402,-336,-16958,-19030,-10525,-8858,-10490,-15430,2577,-14257,985,7038,-20,832,-3382,-13259,-392,8592,15544,-3848,-14553,1487,3364,-2030,-10275,18102,-21101,-4561,-11819,-5689,2842,1234,-6356,6948,5565,5789,-3807,9376,5908,22434,9510,10345,473,-11522,-3320,-6777,-5044,4964,-4980,-8005,-3661,-1439,3660,-8519,2837,697,11843,-1907,-1203,-10475,-12388,2776,-2232,4504,-7983,17066,1093,9363,-2030,-1454,6170,4969,-10018,-13707,-9818,-12661,532,-3678,-3291,-12171,6971,3929,-802,9131,-7816,3708,3115,3123,-6522,2922,-4605,-8017,-10761,9280,-7565,-1547,-10809,-2658,-28903,-9721,-12756,-9072,-1908,-2910,17387,-5114,-2236,-14034,-4270,-11965,270,7628,3615,-6266,-14582,13056,18332,1077,-8853,10711,-5300,-4202,7926,-12388,-17499,3761,3583,850,4441,9677,8678,-5085,7243,1549,-4521,-10672,5785,-3163,415,-7091,-13866,1534,-556,10722,7169,9419,-6697,-8374,-11349,4453,24941,6506,-6914,7481,-10091,-12739,6415,-6957,2825,12578,3587,8313,-14360,5915,19123,4839,-13780,-1817,-4471,3425,3684,3626,-3254,-5360,-5273,7632,-16571,-1854,2417,-5838,4267,-638,-8391,1443,-2399,-3666,-6362,549,-16510,-3563,-4861,-8155,5836,-6567,4834,8726,-1403,2484,5718,-725,4836,-11291,9930,-8850,-16355,12380,-561,4014,-3249,-9127,-27481,5600,-2564,-1765,3627,-12693,-6524,10260,5346,-11458,-19703,-5109,1913,8427,838,-10037,2737,-10484,-8594,-5847,-10994,-2424,-8048,-2555,-4136,-3665,-4503,11028,5457,528,-13119,-14539,-17570,23297,8695,2298,-8713,-10252,4897,13995,-15143,-4019,-754,-17814,7522,-3709,17099,-3496,3659,-10217,-5588,-10452,3510,11641,-5309,3902,-4511,913,10697,1075,-9273,-7665,-1520,-1715,-7743,1615,11325,-2247,-1830,-19688,-6927,-12460,-3454,4700,13621,16083,-1046,-2864,7310,4383,6843,6959,-13238,4832,-819,-6799,-6203,11459,-7242,-11061,1919,17981,228,2756,-8391,14971,1117,4740,-8514,7637,3479,-10338,355,-4030,976,-2241,-1699,-9237,-2892,6854,15601,-10379,21260,13647,1506,-2694,-6248,7447,-6212,-4131,7901,3898,8019,4513,-3586,-4596,7121,-6003,10611,7070,2572,28,3269,31537,-1006,-13920,17125,5453,-5794,23405,4926,2630,-6956,-9524,-15815,-3782,-9418,5792,-2841,15819,13508,1466,-2529,10303,-10253,6024,-5984,-3580,-6008,5147,-6959,-9511,4945,-3787,3233,-3147,2410,-22933,4166,16575,9293,8145,-2144,1618,2184,861,-7427,-3151,-1563,-4299,7483,22699,-11834,1147,-12899,11647,12608,-14868,17381,-9700,2028,9827,1316,20529,24614,1236,-17055,-3264,-7585,-6839,-10085,699,18838,-6402,-2509,10431,10845,4341,9175,7786,-3414,3835,-2675,-8048,5676,603,-1674,2608,8832,3513,1739,2501,458,-6926,7035,12599,-4777,17239,-1577,-2180,-8027,-7086,-8523,3164,14281,4716,7638,11999,-22353,-1499,3156,6414,-11669,-8996,7070,-1043,4946,-8591,-14611,6807,10671,14594,-13702,-11955,-13180,-373,-5505,11751,-13013,-15582,1835,10667,-7180,-11319,-1336,-14635,155,7407,1947,-17074,-4801,-2701,16218,8181,-511,11709,-9343,-3115,12631,13435,6531,4388,-5242,6661,-7406,5137,2520,10188,-3809,5688,7093,-8034,1397,14283,1005,-2760,4083,-2461,3962,-1206,-164,4120,12182,4040,14740,-18139,-4018,2110,-7469,-11411,5517,5479,2587,3535,-7786,-4451,-954,-6151,2967,21151,-4313,5789,-9251,11979,-8667,1866,-5532,4578,101,-394,26372,-5635,-420,23993,-10255,4945,-9627,9216,2928,5831,-2494,2192,13168,-20346,-14606,3715,-5856,-2651,-13387,-8092,-3615,-6491,-7749,-3773,-8430,11999,-8840,16257,-3349,-12983,-5533,8356,9442,1432,2570,5659,-13048,-5202,-16367,-4017,8488,6415,20459,1490,-19285,15107,11466,-5211,1990,6970,3440,6227,-1008,-348,787,-7062,12724,56,6138,-7646,-9616,-814,-2261,10684,5419,4833,-12922,-8654,1806,-3111,11536,11993,-5194,7825,12478,2870,14517,9500,1914,7840,1738,-3711,3205,323,-3261,15837,1978,24416,-2649,5046,14152,24406,2714,-7067,7184,-11804,-2449,2431,13320,12851,-247,8262,-2874,5912,17127,1401,-2688,-4472,6407,11958,19009,483,1457,-5659,14415,-675,-4232,19532,7242,6405,-8993,3880,4651,-9768,-2020,-3622,4720,-9005,9738,15961,-2717,-78,4530,10760,4668,3550,-4318,-2071,5485,15048,5082,-10785,3872,-824,-2183,-1961,-7868,-2869,-7015,-3264,1048,1559,-1929,-1365,301,4098,11461,5545,-2564,5347,-2196,-15925,-20991,-15324,-2121,-5542,-6441,363,-5893,-5639,5451,6996,21789,2704,521,-5135,-1746,-452,-15702,-2302,6702,-5105,4419,8866,9632,6946,-20209,-5047,8061,3529,43,3907,10105,1375,-6785,-1612,-1859,8019,3687,4896,1322,-3240,546,1937,-12503,1599,8294,-986,14061,5012,-3759,-1377,-2461,2156,6748,-2611,4099,15696,8329,7379,-7287,-4778,2169,-901,-14533,-13543,9176,-6780,18585,-19871,4013,5,-6763,3615,-7100,7688,597,-14661,-21684,-2539,10246,1620,485,6150,-12461,12746,-7994,337,-3252,1337,-17389,13627,4880,17973,12668,102,-8398,-15551,151,1959,9352,-8505,7107,638,-6919,6922,2375,-2103,16787,5448,-987,2481,778,1552,4056,-5397,-7128,3225,16857,-2744,-9057,-2134,-4329,-2928,4252,-1163,-5316,-3969,-11827,-7687,-12636,-4615,-9704,-6324,-2698,-10736,-174,3043,-8614,9961,-14184,-3504,-13277,-3082,-11840,-5141,-6624,-10597,-18466,5053,17886,19882,-4402,-412,-4165,679,-8217,-17165,-326,-10956,-17013,21231,-2087,3611,10663,-15067,3694,4486,742,1411,-4722,6466,-7601,-7122,6486,15082,-3456,-4517,3649,9589,8656,2415,-5674,5631,-709,12313,-12594,1262,11533,-4426,-5588,-8350,-2493,-1794,1222,-7900,738,-5401,-3298,-7496,-2527,31923,30472,10256,7040,-11430,-5228,-5540,4786,4319,970,-3044,-8280,47,10186,3821,1618,5879,5224,-14456,167,-3810,-18190,-11881,-2861,7381,-2051,2345,-8031,-19292,839,-8506,10485,15516,-3324,10667,8528,-12655,-299,2335,4676,6052,6030,6066,-2796,3391,-20147,-10449,10748,-5909,2929,13486,-7950,-8579,-4407,3944,-1832,-8745,-14067,-3312,-1978,-4787,-2343,-9796,4911,-19447,1293,5982,-7591,-511,2609,-5497,-123,3685,-8381,7468,9787,2872,135,-10414,4425,-10873,14225,4708,2368,-3520,-19462,-10693,3266,21573,6619,2769,-4847,1093,8605,-2272,16917,-10913,-3365,468,-17474,5056,695,721,7106,9333,7173,-4193,-417,3160,-11240,-9265,-3819,694,16015,7129,-3958,-5626,13716,24444,1497,-4283,-12633,8718,2590,10421,-8127,2151,-5849,11401,-4930,-782,-3111,8888,3831,-8980,-5658,-9352,4522,2968,-14625,-17053,-362,6234,1509,-13806,-13878,7753,-1301,-3454,11770,-7123,1211,3736,7331,-7641,3195,24578,-13530,3880,-2055,-7397,-7441,4452,20707,-7099,4837,-5000,17675,4870,-1234,5543,-50,9901,-1660,-10029,2203,13959,-10701,-2168,8985,-17171,5593,6733,1909,-6888,-64,833,8363,5933,-3132,14485,-455,-7268,-3921,7678,1745,7948,619,22238,-14896,-3720,-754,798,13021,1962,-1028,613,6720,-6166,4031,-13996,-705,-8423,-5839,2526,-10062,-8844,-13556,-19929,-1355,10397,-1627,-2093,-9361,14158,-2231,11571,3237,-2353,9677,8785,-997,11589,15929,-587,-2287,3618,3145,7221,-2286,6394,9377,1587,1370,-11217,-10488,-13070,-11128,-1387,-14442,20085,6611,-18491,-10325,-959,-15318,-4113,9756,-1466,-602,-8957,-4970,-12087,3256,-4044,7005,11064,9632,-312,-4507,-10946,1059,8580,-3049,-1550,5511,7725,-12671,1788,1751,-1352,7983,-3257,-7389,2468,4111,-1946,7119,-13575,-9613,-27474,4662,-8863,-2264,9025,456,-3939,-7588,-2150,5391,-13560,-612,6996,-12705,5985,6109,-7889,-13455,3873,7227,5177,6821,-10572,5221,-5187,-5021,-13914,9838,1565,8974,13512,-10170,5750,-115,8175,9905,7337,-7312,-1132,2362,28245,10969,20752,3706,1894,5483,-4720,11103,-1410,-12284,7787,-14027,-16493,2579,-8504,-8145,-1449,-4373,-1993,2432,-10450,-10948,-18775,-3490,5940,-6281,4483,4831,8859,8847,-6160,-7663,432,-5946,12991,12339,850,-3820,4567,-5869,-1118,-4744,944,10090,6639,10226,-8180,1607,-8794,3441,2912,11592,9829,5841,-4517,-4257,-18347,-4009,-13872,8315,8065,1236,-3384,1279,14353,12064,-4011,1506,-10021,3583,6609,8048,-13693,4512,-7729,-3368,7046,2666,-1465,5425,14616,-5576,-12074,-10394,-8445,-6003,5157,-18653,2110,-6962,9832,-7651,67,-8385,-6095,-2327,-2047,-4694,10090,4919,16589,-2480,9538,-18773,5709,3321,-3347,6218,7848,9243,3754,5375,-6028,-9794,-2394,1668,8503,-7070,-4378,26599,-5568,17172,8595,-4989,-953,-1925,4234,12212,-14647,18097,6955,-4909,-1127,2679,2648,10738,9748,-3208,-1162,6561,1076,10161,-6142,4218,2578,12446,-12033,6,478,-20943,11161,25121,-2077,2566,-4155,3436,-3397,-911,14470,-7577,-5150,-679,-1185,12871,18164,-8748,-5733,4654,-3405,17821,14628,4999,5040,-3027,6828,10082,1266,17242,5997,-985,3317,9806,-3561,-765,21223,-4239,8462,7316,14209,4703,-22046,-7624,4576,2305,17549,12576,4442,740,-13848,16661,1198,-13826,3874,920,-5101,4409,-6327,-12055,-15688,-3254,-5608,3936,-13438,-1863,-13460,16183,-1047,10965,4426,8125,-10322,11245,12989,-18572,-1579,-5832,1399,-7604,2495,5339,2188,-1059,-11531,514,-4659,-11438,-503,-1876,1717,8873,5153,11677,-6522,-7732,-849,12369,11658,-1126,-5338,-13599,6007,204,-10533,-2992,-16360,3845,-1556,14587,-12157,8678,1284,738,6408,10672,-9579,11794,-10821,-9606,-6015,6585,-9569,2993,3683,1378,4959,-10489,9010,1015,6528,-8796,465,7847,9069,-755,-16576,-9809,1955,7102,4141,-5479,20070,646,7739,-3718,-9971,6127,9279,16295,-4731,-14559,14462,2362,-10080,-5001,-7234,10382,5294,2170,2150,-7347,8880,3097,-2336,-1652,-910,-7933,6629,12449,22122,4506,-7356,1797,-9006,-5484,-5893,-7452,3381,-9975,5966,7112,9277,-183,5535,16123,473,-1591,15850,-22436,-4084};

int REFRESH_RATE_HZ = 6000;
int SAMPLE_RATE_HZ = 44100; //2757;            // Sample rate for getting data- because of the audio shield, it's now 16samples in a go. The updates have been set

const int FFT_SIZE = 1024;              // Size of the FFT. This is only 256 from now on.
  
const int BUFFER_SIZE = FFT_SIZE * 4;  // Create a buffer than can overrun.
const int TEMPLT_SIZE = FFT_SIZE  / 2;

int AVERAGING = 2; // Number of attempts to get one new block of samples (updated ever 0.35ms) for averaging FF estimates. Increased accuracy at cost of 0.35ms per averaging cycle.
// Note that this is _only_ for FF estimate after a template match occurs.

///////////////// White Noise Output Stuff.
const int BNC_TRIGGER_OUTPUT_PIN = 39;    // Pin 39 for Teensy3.6. --> BNC
const int MAX_CHARS = (TEMPLT_SIZE * 30) + 15;


///////////////////////Templates and Triggers..
// The 4 is fast enough to spit out it's PSD calculations. To get an outline of what song looks like on the Teensy,
//Template for matching....
float templt[TEMPLT_SIZE] = {0,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,1,1,1,1,1,1,1,1,1,1,1,1,-5,-5,-5,-5,-5,-5,-5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-5,-5,-0.500000000000000,-0.500000000000000,-5,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000};
float templtpre[TEMPLT_SIZE] = {0,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,1,1,1,1,1,1,1,1,1,1,1,1,-5,-5,-5,-5,-5,-5,-5,3,3,3,3,3,3,3,3,3,3,3,3,3,3,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-5,-5,-0.500000000000000,-0.500000000000000,-5,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000,-0.500000000000000};
// Thresholding Variables...
int AMP_THRESHOLD = 900; // Amplitute Threshold (below this value, we don't do template matching.... Otherwise background noise can trigger a template match by chance once it's normalized
int AMP_MAX = 12000;
int LINE_IN_LEVEL=55; //dB for Mic Input. 
int TIME_BETWEEN_TEMPLATES = 5;
float DPTHRESH = 50;      // Value for Template Match Trigger.
float PREDPTHRESH = -50;      // Value for Template Match Trigger.
int AMPLITUDE_THRESHOLD_DURATION = 50; //how many samples in a row should be above threshold. value depends on other parameters.
int DUR_MATCH_TEMPLATE = 3; // How long should the template be above threshold? Number of FFTs in a row that need to match....
int PRE_DUR_MATCH_TEMPLATE = 2; // Pre count for FFT
float FF_DISCARD = 300; // Ignore parts of the FF below this (200-300Hz usually fine).
float FF_MIN = 2900; //  Lower Limit for frequency bins to find peak. // lower than 500 is a bad plan.
float FF_MAX = 4500; //. Upper Limit to look at frequencies to find peak.
int FREQTHRESH = 3800 ;  /////////// Frequency Limits
int CATCHPERCENT = 100; /////////  Maintain this percent catch trials.
boolean isRunning = false;
boolean isDIR = false;
elapsedMillis delaytime;
int MINDELAY = 10;
int MAXDELAY = 500;
boolean TESTPRETEMPLATE = true;
/////// Useful variables defined.
float dp;  // dot product for template matching.
int16_t scaledMag;
float FF; //estimated FF

int peakIDX;
int PlayBackCounter;
boolean PLAYWN = true; // WN or or off? Used for testing.
boolean FREQDIR = true; // 1=up, 0=down (true=up, false=down)
float FFT_Bin = (float)((SAMPLE_RATE_HZ / 2) / (FFT_SIZE / 2)); // Fixed (real) sampling rate now.
int FF_discardIDX = (int)(FF_DISCARD / FFT_Bin);
int ThreshRand = 0; // Random variable for generating Hit/Catch trials
int FF_maxIDX; //= (int)(FF_MAX / FFT_Bin); //float to int! ////////// Indexes for finding peak frequency after template math. Offset for Parabolic interpolation.
int FF_minIDX; //= (int)(FF_MIN / FFT_Bin);                  /////// Lower bound for detection. -1 because of offset issues, make sure we include that peak + 1 bin for interpolation.

////////////////////////////////////////////////////////////////////////////////
// INTERNAL STATE
// These shouldn't be modified unless you know what you're doing.
////////////////////////////////////////////////////////////////////////////////

IntervalTimer samplingTimer;
volatile int16_t samples[FFT_SIZE * 8];
int16_t magnitudes[FFT_SIZE];
int16_t fftbuffer[FFT_SIZE * 2];
volatile bool SamplesAvailable;
volatile int sampleCounter = 0;

char commandBuffer[MAX_CHARS];
boolean PLOTMAGS=false;
//boolean WNTRIG=false;
//boolean WNCATCH=false;
int AboveThresh = 0;
int AboveThreshold = 0;
int fft_threshcounter = 0;
int fft_threshcounter_pre = 0;
int dpold = 0;
int HIT = 1;
int16_t RUNNING_AMP = 0;

// From TeensyAudio library.
//const int16_t AudioWindowHanning256[] __attribute__ ((aligned (4))) = {
//     0,     5,    20,    45,    80,   124,   179,   243,   317,   401,
//   495,   598,   711,   833,   965,  1106,  1257,  1416,  1585,  1763,
//  1949,  2145,  2349,  2561,  2782,  3011,  3249,  3494,  3747,  4008,
//  4276,  4552,  4834,  5124,  5421,  5724,  6034,  6350,  6672,  7000,
//  7334,  7673,  8018,  8367,  8722,  9081,  9445,  9812, 10184, 10560,
// 10939, 11321, 11707, 12095, 12486, 12879, 13274, 13672, 14070, 14471,
// 14872, 15275, 15678, 16081, 16485, 16889, 17292, 17695, 18097, 18498,
// 18897, 19295, 19692, 20086, 20478, 20868, 21255, 21639, 22019, 22397,
// 22770, 23140, 23506, 23867, 24224, 24576, 24923, 25265, 25602, 25932,
// 26258, 26577, 26890, 27196, 27496, 27789, 28076, 28355, 28627, 28892,
// 29148, 29398, 29639, 29872, 30097, 30314, 30522, 30722, 30913, 31095,
// 31268, 31432, 31588, 31733, 31870, 31997, 32115, 32223, 32321, 32410,
// 32489, 32558, 32618, 32667, 32707, 32737, 32757, 32767, 32767, 32757,
// 32737, 32707, 32667, 32618, 32558, 32489, 32410, 32321, 32223, 32115,
// 31997, 31870, 31733, 31588, 31432, 31268, 31095, 30913, 30722, 30522,
// 30314, 30097, 29872, 29639, 29398, 29148, 28892, 28627, 28355, 28076,
// 27789, 27496, 27196, 26890, 26577, 26258, 25932, 25602, 25265, 24923,
// 24576, 24224, 23867, 23506, 23140, 22770, 22397, 22019, 21639, 21255,
// 20868, 20478, 20086, 19692, 19295, 18897, 18498, 18097, 17695, 17292,
// 16889, 16485, 16081, 15678, 15275, 14872, 14471, 14070, 13672, 13274,
// 12879, 12486, 12095, 11707, 11321, 10939, 10560, 10184,  9812,  9445,
//  9081,  8722,  8367,  8018,  7673,  7334,  7000,  6672,  6350,  6034,
//  5724,  5421,  5124,  4834,  4552,  4276,  4008,  3747,  3494,  3249,
//  3011,  2782,  2561,  2349,  2145,  1949,  1763,  1585,  1416,  1257,
//  1106,   965,   833,   711,   598,   495,   401,   317,   243,   179,
//   124,    80,    45,    20,     5,     0,
//};
//
//
//const int16_t AudioWindowHanning1024[] __attribute__ ((aligned (4))) = {
//     0,     0,     1,     3,     5,     8,    11,    15,    20,    25,
//    31,    37,    44,    52,    61,    69,    79,    89,   100,   111,
//   123,   136,   149,   163,   178,   193,   208,   225,   242,   259,
//   277,   296,   315,   335,   356,   377,   399,   421,   444,   468,
//   492,   517,   542,   568,   595,   622,   650,   678,   707,   736,
//   767,   797,   829,   860,   893,   926,   960,   994,  1029,  1064,
//  1100,  1137,  1174,  1211,  1250,  1288,  1328,  1368,  1408,  1449,
//  1491,  1533,  1576,  1619,  1663,  1708,  1753,  1798,  1844,  1891,
//  1938,  1986,  2034,  2083,  2133,  2182,  2233,  2284,  2335,  2387,
//  2440,  2493,  2547,  2601,  2656,  2711,  2766,  2823,  2879,  2937,
//  2994,  3053,  3111,  3171,  3230,  3291,  3351,  3413,  3474,  3536,
//  3599,  3662,  3726,  3790,  3855,  3920,  3985,  4051,  4118,  4185,
//  4252,  4320,  4388,  4457,  4526,  4596,  4666,  4737,  4808,  4879,
//  4951,  5023,  5096,  5169,  5243,  5317,  5391,  5466,  5541,  5617,
//  5693,  5769,  5846,  5923,  6001,  6079,  6158,  6236,  6316,  6395,
//  6475,  6555,  6636,  6717,  6799,  6880,  6962,  7045,  7128,  7211,
//  7295,  7379,  7463,  7547,  7632,  7717,  7803,  7889,  7975,  8062,
//  8148,  8236,  8323,  8411,  8499,  8587,  8676,  8765,  8854,  8944,
//  9033,  9123,  9214,  9304,  9395,  9486,  9578,  9670,  9761,  9854,
//  9946, 10039, 10132, 10225, 10318, 10412, 10505, 10599, 10694, 10788,
// 10883, 10978, 11073, 11168, 11264, 11359, 11455, 11551, 11648, 11744,
// 11841, 11937, 12034, 12131, 12229, 12326, 12424, 12521, 12619, 12717,
// 12815, 12914, 13012, 13111, 13209, 13308, 13407, 13506, 13605, 13704,
// 13804, 13903, 14003, 14102, 14202, 14302, 14401, 14501, 14601, 14701,
// 14802, 14902, 15002, 15102, 15203, 15303, 15403, 15504, 15604, 15705,
// 15806, 15906, 16007, 16107, 16208, 16309, 16409, 16510, 16610, 16711,
// 16812, 16912, 17013, 17113, 17214, 17314, 17415, 17515, 17616, 17716,
// 17816, 17916, 18017, 18117, 18217, 18317, 18416, 18516, 18616, 18716,
// 18815, 18915, 19014, 19113, 19213, 19312, 19411, 19509, 19608, 19707,
// 19805, 19904, 20002, 20100, 20198, 20296, 20393, 20491, 20588, 20685,
// 20782, 20879, 20976, 21072, 21169, 21265, 21361, 21457, 21552, 21647,
// 21743, 21838, 21932, 22027, 22121, 22216, 22309, 22403, 22497, 22590,
// 22683, 22776, 22868, 22961, 23053, 23144, 23236, 23327, 23418, 23509,
// 23599, 23690, 23780, 23869, 23959, 24048, 24136, 24225, 24313, 24401,
// 24489, 24576, 24663, 24750, 24836, 24922, 25008, 25093, 25178, 25263,
// 25347, 25431, 25515, 25599, 25682, 25764, 25847, 25929, 26010, 26091,
// 26172, 26253, 26333, 26413, 26492, 26571, 26650, 26728, 26806, 26883,
// 26960, 27037, 27113, 27189, 27265, 27340, 27414, 27488, 27562, 27636,
// 27708, 27781, 27853, 27925, 27996, 28067, 28137, 28207, 28276, 28345,
// 28414, 28482, 28550, 28617, 28683, 28750, 28815, 28881, 28946, 29010,
// 29074, 29137, 29200, 29263, 29325, 29386, 29447, 29508, 29568, 29627,
// 29686, 29745, 29803, 29860, 29917, 29974, 30029, 30085, 30140, 30194,
// 30248, 30301, 30354, 30407, 30458, 30510, 30560, 30611, 30660, 30709,
// 30758, 30806, 30853, 30900, 30947, 30993, 31038, 31083, 31127, 31170,
// 31213, 31256, 31298, 31339, 31380, 31420, 31460, 31499, 31538, 31576,
// 31613, 31650, 31686, 31722, 31757, 31791, 31825, 31859, 31891, 31924,
// 31955, 31986, 32017, 32046, 32076, 32104, 32132, 32160, 32187, 32213,
// 32239, 32264, 32288, 32312, 32335, 32358, 32380, 32402, 32422, 32443,
// 32462, 32481, 32500, 32518, 32535, 32551, 32567, 32583, 32598, 32612,
// 32625, 32638, 32651, 32662, 32673, 32684, 32694, 32703, 32712, 32720,
// 32727, 32734, 32740, 32746, 32751, 32755, 32759, 32762, 32764, 32766,
// 32767, 32767, 32767, 32767, 32766, 32764, 32762, 32759, 32755, 32751,
// 32746, 32740, 32734, 32727, 32720, 32712, 32703, 32694, 32684, 32673,
// 32662, 32651, 32638, 32625, 32612, 32598, 32583, 32567, 32551, 32535,
// 32518, 32500, 32481, 32462, 32443, 32422, 32402, 32380, 32358, 32335,
// 32312, 32288, 32264, 32239, 32213, 32187, 32160, 32132, 32104, 32076,
// 32046, 32017, 31986, 31955, 31924, 31891, 31859, 31825, 31791, 31757,
// 31722, 31686, 31650, 31613, 31576, 31538, 31499, 31460, 31420, 31380,
// 31339, 31298, 31256, 31213, 31170, 31127, 31083, 31038, 30993, 30947,
// 30900, 30853, 30806, 30758, 30709, 30660, 30611, 30560, 30510, 30458,
// 30407, 30354, 30301, 30248, 30194, 30140, 30085, 30029, 29974, 29917,
// 29860, 29803, 29745, 29686, 29627, 29568, 29508, 29447, 29386, 29325,
// 29263, 29200, 29137, 29074, 29010, 28946, 28881, 28815, 28750, 28683,
// 28617, 28550, 28482, 28414, 28345, 28276, 28207, 28137, 28067, 27996,
// 27925, 27853, 27781, 27708, 27636, 27562, 27488, 27414, 27340, 27265,
// 27189, 27113, 27037, 26960, 26883, 26806, 26728, 26650, 26571, 26492,
// 26413, 26333, 26253, 26172, 26091, 26010, 25929, 25847, 25764, 25682,
// 25599, 25515, 25431, 25347, 25263, 25178, 25093, 25008, 24922, 24836,
// 24750, 24663, 24576, 24489, 24401, 24313, 24225, 24136, 24048, 23959,
// 23869, 23780, 23690, 23599, 23509, 23418, 23327, 23236, 23144, 23053,
// 22961, 22868, 22776, 22683, 22590, 22497, 22403, 22309, 22216, 22121,
// 22027, 21932, 21838, 21743, 21647, 21552, 21457, 21361, 21265, 21169,
// 21072, 20976, 20879, 20782, 20685, 20588, 20491, 20393, 20296, 20198,
// 20100, 20002, 19904, 19805, 19707, 19608, 19509, 19411, 19312, 19213,
// 19113, 19014, 18915, 18815, 18716, 18616, 18516, 18416, 18317, 18217,
// 18117, 18017, 17916, 17816, 17716, 17616, 17515, 17415, 17314, 17214,
// 17113, 17013, 16912, 16812, 16711, 16610, 16510, 16409, 16309, 16208,
// 16107, 16007, 15906, 15806, 15705, 15604, 15504, 15403, 15303, 15203,
// 15102, 15002, 14902, 14802, 14701, 14601, 14501, 14401, 14302, 14202,
// 14102, 14003, 13903, 13804, 13704, 13605, 13506, 13407, 13308, 13209,
// 13111, 13012, 12914, 12815, 12717, 12619, 12521, 12424, 12326, 12229,
// 12131, 12034, 11937, 11841, 11744, 11648, 11551, 11455, 11359, 11264,
// 11168, 11073, 10978, 10883, 10788, 10694, 10599, 10505, 10412, 10318,
// 10225, 10132, 10039,  9946,  9854,  9761,  9670,  9578,  9486,  9395,
//  9304,  9214,  9123,  9033,  8944,  8854,  8765,  8676,  8587,  8499,
//  8411,  8323,  8236,  8148,  8062,  7975,  7889,  7803,  7717,  7632,
//  7547,  7463,  7379,  7295,  7211,  7128,  7045,  6962,  6880,  6799,
//  6717,  6636,  6555,  6475,  6395,  6316,  6236,  6158,  6079,  6001,
//  5923,  5846,  5769,  5693,  5617,  5541,  5466,  5391,  5317,  5243,
//  5169,  5096,  5023,  4951,  4879,  4808,  4737,  4666,  4596,  4526,
//  4457,  4388,  4320,  4252,  4185,  4118,  4051,  3985,  3920,  3855,
//  3790,  3726,  3662,  3599,  3536,  3474,  3413,  3351,  3291,  3230,
//  3171,  3111,  3053,  2994,  2937,  2879,  2823,  2766,  2711,  2656,
//  2601,  2547,  2493,  2440,  2387,  2335,  2284,  2233,  2182,  2133,
//  2083,  2034,  1986,  1938,  1891,  1844,  1798,  1753,  1708,  1663,
//  1619,  1576,  1533,  1491,  1449,  1408,  1368,  1328,  1288,  1250,
//  1211,  1174,  1137,  1100,  1064,  1029,   994,   960,   926,   893,
//   860,   829,   797,   767,   736,   707,   678,   650,   622,   595,
//   568,   542,   517,   492,   468,   444,   421,   399,   377,   356,
//   335,   315,   296,   277,   259,   242,   225,   208,   193,   178,
//   163,   149,   136,   123,   111,   100,    89,    79,    69,    61,
//    52,    44,    37,    31,    25,    20,    15,    11,     8,     5,
//     3,     1,     0,     0,
//};
