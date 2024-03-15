//
// Created by Noah Holbrook on 1/14/24.
//

#ifndef AMETHYST_CHESS_ZOBRISTHASHING_H
#define AMETHYST_CHESS_ZOBRISTHASHING_H

namespace zobrist_hashing {
    const static unsigned long WHITE_KING_CODES[64] = {40488560587228803, 202443102859642738, 435755061918289796, 449393352604679852, 517584806036630132, 282081320892958915, 257485399781448064, 134505794223893809, 96068518739545151, 480342893621224478, 95871758815930645, 479359094003151948, 90952760725567995, 454764103551338698, 544438560769924362, 416350094559430065, 352368515810381197, 32460622065136857, 162303410249183008, 235056598865991146, 22361789646609219, 111809248156544818, 559046540706222813, 489389994240922320, 141107261914419855, 129075857192175381, 68918833580953011, 344594467828263778, 570051134457972379, 544412962999670150, 416222105708159005, 351728571554025897, 29260900783360357, 146304803840300508, 155063566821578646, 198857381727969336, 417826456259922786, 359750324312844802, 69369664577454882, 346848622810773133, 4861157067096537, 24306085258981408, 121530726218405763, 31193178712104921, 155966193484023328, 203370515040192746, 440392122821039836, 472578657118430052, 57050576301958515, 285253181433291298, 273344702483109979, 213802307732203384, 492551086281093026, 156912722115273385, 208103158196443031, 464055338602291261, 14433983721264560, 72170218529821523, 360851392572606338, 74875005876262562, 374375329304811533, 142494689537288537, 136012995306518791, 103604524152670061};
    const static unsigned long BLACK_KING_CODES[64] = {510293633595693942, 245625458688277965, 75206088758043314, 376030743713715293, 150771761581807337, 177398355529112791, 310531325265640061, 399735421644853794, 269295151237499842, 193554551504152699, 391312305140839601, 227179568717428877, 559437391207220491, 491344246745910710, 150878524439361805, 177932169816885131, 313200396704501761, 413080778839162294, 336021937209042342, 527188481361865199, 330099697519134250, 497577282912324739, 182043705271431950, 333758073977235856, 515869165202832769, 273503116723972100, 214594378936513989, 496511442302646051, 176714502223038510, 307112058735268656, 382639088992996769, 183813487978214717, 342606987511149691, 560113732872401944, 494725955071817975, 167787066068898130, 262474877964566756, 159453185139487269, 220805473317512451, 527566914207638361, 331991861748000060, 507038104056653789, 229347810993077200, 570278602585462106, 545550303637118785, 421908808895402180, 380162087490241772, 171428480464439732, 280681949942274766, 250488545028027319, 99521520456790084, 497607902207449143, 182196801747053970, 334523556355345956, 519696577093383269, 292640176176724600, 310279676200276489, 398477176318035934, 263003924603410542, 162098418333706199, 234031639288607101, 17236991759688994, 86185258721943693, 430926593533217188};
    const static unsigned long WHITE_PIECE_TYPE_CODES[5][64] = {
            {403637954300736464, 288807814516913192, 291117867901219449, 302668134822750734, 360419469430407159, 72715390165266667, 363577250749832058, 88504296762391162, 442521783735454533, 483226961690503537, 110292099162325940, 551460795735128423, 451461269385450370, 527924389940482722, 333779240412221865, 515974997377762814, 274032277598622325, 217240183309765114, 509740464168901676, 242859611554316635, 61376853088236664, 306884565364682043, 381501622140063704, 178126153713549392, 314170316187823066, 417930376255768819, 360269924292074967, 71967664473605707, 359838622291527258, 69811154470867162, 349056072277834533, 15898404402403537, 79492321935516408, 397461909601080763, 257927591018634687, 136716750409826924, 107123299669210726, 535616798269552353, 372241282057570020, 131824453301080972, 82661814125480966, 413309370550903553, 337164895767748637, 532903274155396674, 358673661486791625, 63986350447188997, 319932052159443708, 446739056113872029, 504313323582591017, 215723908622763340, 502159090733892806, 204952744379272285, 448303269516437531, 512134390595418527, 254829243686900890, 121225013751157939, 29664616375865801, 148323381802827728, 165156456634214746, 249321830791149836, 93687949272402669, 468440046285512068, 36357522137368595, 181787910610341698},
            {296982275005778986, 331990170345548419, 507029647044395584, 229305525931786175, 570067177279006981, 544493177104843160, 416623176234024055, 353733924183351147, 39287663929986607, 196438619573431758, 405732645487234896, 299281270449405352, 343485147563680249, 564504533135054734, 516679956385081925, 277557072635217880, 234864158492742889, 21399587780367934, 106998238825338393, 534991494050190688, 369114760960761695, 116191847817039347, 4498786705272841, 22494233449862928, 112471467172813363, 562357635787565538, 505945469647635945, 223884638947987980, 542962742360016006, 408971002509888285, 315473055562672297, 424444073130014974, 392838408663305742, 234810086329759582, 21129226965451399, 105646434750755718, 528232473677277313, 335319659096194820, 523677090797627589, 312542744697946200, 409792518806384489, 319580637045153317, 444981980542420074, 495527945725331242, 171797019336464465, 282524644302398431, 259702016828645644, 145588879459881709, 151483944919484651, 180959272217499361, 328335908707572911, 488758338854518044, 137948984982398475, 113284472532068481, 566422662583841128, 526270603629013895, 325510308854877730, 474630339591042139, 67308988665018950, 336545243248593473, 529805011559620854, 343182348507912525, 562990537856216114, 509109979990888825},
            {190326595710821508, 375172526174183646, 146480673884149102, 155942917040821616, 203254132824184186, 439810211740997036, 469669101718216052, 42502799300888515, 212514296427941298, 486111029759782596, 124712439508721235, 47101745163682281, 235509025741910128, 24623924026204129, 123119920054519368, 39139147892672946, 195696039386863453, 402019744554393371, 280716765785197727, 250662624242642124, 100391916529864109, 501959882572819268, 203956703573904595, 443323065489599081, 487233370461226277, 130324143015939640, 75160262699774306, 375801613422370253, 149626110125082137, 171670098245486791, 281890038847510061, 256528989554203794, 129723743087672459, 72158263058438401, 360791615215690728, 74576119091684512, 372880895381921283, 135022519922837287, 98652147234262541, 493261036094811428, 160462471183865395, 225851903539403081, 552799065317091511, 458152617295265810, 561381129489559922, 501062938157607865, 199471981497847580, 420899455109314006, 375115318559800902, 146194635812235382, 154512726681253016, 196103181026341186, 404055452751782036, 290895306772141052, 301555329177358749, 354855441203447234, 44895249030467042, 224476545075833933, 545922272999245771, 423768655706037110, 389461321543416422, 217924650730312982, 513162801271641016, 259971297068013335},
            {83670916415864030, 418354882002818873, 362392453027325237, 82580308149857057, 412901840672784008, 335127246377150912, 522715027202408049, 307732426721848500, 385740928925895989, 199322687642710817, 420152985833630191, 371382972181381827, 127532903920140007, 61204067220776141, 306020636027379428, 377181975453550629, 156527920280984017, 206179149024996191, 454435292745057061, 542794506738516177, 408129824402389140, 311267165025176572, 403414620442536349, 287691145225912617, 285534521446216574, 274751402547736359, 220835808055335284, 527718587896752526, 332750230193570885, 510829946284507914, 248307022132347825, 88613905978392614, 443069829815461793, 485967192090539837, 123993251162507440, 43505803432613306, 217529317086565253, 511186133052902371, 250087955974320110, 97518575188254039, 487593175864768918, 132123170033652845, 84155397788340331, 420777288865200378, 374504487339232762, 143140479709394682, 139241946167049516, 119749278455323686, 22285939896694536, 111429999406971403, 557150296958355738, 479908775501586945, 93701168217742980, 468506141012213623, 36687995770876370, 183440278777880573, 340740941509478971, 550783502864048344, 448074805030049975, 510992068163480747, 249117631527211990, 92666952952713439, 463335064687065918, 10832614145137845},
            {553475989424329169, 461537237831454100, 1843479867078755, 9217699258892498, 46088796217961213, 230444281013304788, 575760952686600046, 572962054142808485, 558967561423850680, 488995097829061655, 139132779855116530, 119203446895658756, 19556782098369886, 97784210415348153, 488921352000239488, 138764050711005695, 117359801175104581, 10338553495599011, 51693067401493778, 258465636930967613, 139406979971491554, 120574447477533876, 26411785007745486, 132059224962226153, 83835672431206871, 419178662079533078, 366511353410896262, 103174810067712182, 515874350262059633, 273529042020106420, 214724005417185589, 497159574706004051, 179955164239828510, 323315368819218656, 463655639412746769, 12435487773542100, 62177738791209223, 310888993879544838, 401523764714377679, 278236866585119267, 238263128242249824, 38394436527902609, 191972482563011768, 383401960435134946, 187627845188905602, 361678773564604116, 79011910836251452, 395059854104755983, 245917313537010787, 76665363001707424, 383327114932035843, 187253617673410087, 359807635987126541, 69656222948863577, 348281414667816608, 12025116352313912, 60125881685068283, 300629708348840138, 350227337060854179, 21754728317501767, 108773941511007558, 543870007478536513, 413507328102490820, 338154683525684972}
    };

    const static unsigned long BLACK_PIECE_TYPE_CODES[5][64] = {
            {446820310129371691, 504719593660089327, 217755259010254890, 512315842671350556, 255736504066561035, 125761315649458664, 52346125867369426, 261730929260345853, 155733441618382754, 202206755711989876, 434573326180025486, 443484673913358302, 488041412580022382, 134364353609920165, 95361315669676931, 476806878271883378, 78191682069225145, 390958710269624448, 225411594361353112, 550597519426841666, 447144887844016585, 506342482233313797, 225869701876377240, 552888057001962306, 458597575719619785, 563605921611329797, 512186898766457240, 255091784542094455, 122537718027125764, 36228137755704926, 181140988702023353, 329244491130192871, 493301250967617844, 160663545547897475, 226857275359563481, 557825924417893511, 483286912799275810, 110591854706187305, 552959573454435248, 458955157981984495, 565393832923153347, 521126455325574990, 299789567337683205, 346026632005069514, 751203038578442, 3756315116390933, 18781875505453388, 93909677450765663, 469548687177327038, 41900726596443445, 209503932905715948, 471059212148655846, 49453351453087485, 247267057188936148, 83414081261334229, 417070706230169868, 355971574164080212, 50475913833631932, 252379869091658383, 108978140774945404, 544891003798225743, 418612309700936970, 363679591517915722, 89016000602809482},
            {340164630834414213, 547901949488724554, 433667038153431025, 438953233780385997, 465384211915160857, 21078350285612540, 105392051351561423, 526960556681305838, 328960074116337445, 491879165898340714, 153553120201511825, 191305148627635231, 380065290758252261, 170944496804492177, 278262031642536991, 238388953529338444, 39023562963345709, 195118114740227268, 399130121321212446, 266268649619293102, 178422043413118999, 315649764685671101, 425327618745008994, 397256136738275842, 256898726704610082, 131572428839703899, 81401691818595601, 407008759016476728, 305661838095614512, 375387985794726049, 147557971986861117, 161329407554381691, 230186585391984561, 574472474579998911, 566519663609802810, 526755608758822305, 327935334503919780, 486755467836252389, 127934629891070200, 63212697075427106, 316063785300634253, 427397721819824754, 407606652112354642, 308651303575004082, 390335313191673899, 222294608971600367, 535012592478077941, 369220253100197960, 116719308514220672, 7136090191179466, 35680750879396053, 178404054320478988, 315559819222471046, 424877891429008719, 395007500158274467, 245655543804603207, 75356514339669524, 376782871621846343, 154532401122462587, 196201553232389041, 404547313782021311, 293354611923337427, 313851854933340624, 416338069983356609},
            {233508951539456735, 14623553013937164, 73118064993184543, 365590624889421438, 98571167460338062, 492856137225189033, 158437976835753420, 215729431798843206, 502186706614292136, 205090823781268935, 448993666526420781, 515586375645334777, 272089168936482140, 207524639999064189, 461162747615397051, 576431781090216127, 576316196160888890, 575738271514252705, 572848648281071780, 558400532115167155, 486159951285644030, 124957047138028405, 48324783310218131, 241624216474589378, 55199877689600379, 275999688371500618, 227077237174156579, 558925733490859001, 488785958164103260, 138087081530324555, 113974955271698881, 569875076281993128, 543532672119773895, 411820651308677730, 329721299556619522, 495685293099751099, 172583756208563750, 286458328662894856, 279370438631127769, 243930988472292334, 66733737678115159, 333668988314074518, 515423736887026079, 271275975144938650, 203458671041346739, 440832902826809801, 474782557147279877, 68070076446207640, 340350682154536923, 548832206089338104, 438318321156498775, 462209648795724747, 5205534688431990, 26027973365658673, 130140166751792088, 74240381379036546, 371202206818681453, 126629077106638137, 56684933153266791, 283424965689832678, 264203623765816879, 168096914145737884, 264024118348765526, 167199387060481119},
            {126853272244499257, 57805908842572391, 289029844136360678, 292228015998456879, 308218875308937884, 388173171861342909, 211483902319945417, 480959059219803191, 98952586808824210, 494763233967619773, 167973460547907120, 263406850359611706, 164113047114712019, 244104783193636201, 67602711284834494, 338013856347671193, 537148077055009454, 379897675984855525, 170106422937508497, 274071662307618591, 217437106854746444, 510725081893808326, 247782700178849885, 85992296210902914, 429961780978013293, 420426947903297337, 372752782529717557, 134381955661818657, 95449325929169391, 477246929569345678, 80391938556536645, 401959992706181948, 280418006544140612, 249168828037356549, 92922935503436234, 464614977440679893, 17232177913207720, 86161189489537323, 430806247371185338, 424649279869157562, 393864442359018682, 239940254808324282, 46780069358274899, 233900646714873218, 16582028891019579, 82910444378596618, 414552521816481813, 343380652095639937, 563982055794853174, 514067569684074125, 264495139130178880, 169554490967547889, 271312002457815551, 203638807605731244, 441733585648732326, 479285971256892502, 90587146994270765, 452936034894852548, 535298217487493612, 370648378147276315, 123859933749612447, 42839216368138341, 214196381764190428, 494521456441028246},
            {20197592949541779, 100988264671207618, 504941623279536813, 218865407107492320, 517866583157537706, 283490206497496785, 264529827804137414, 169727934337340559, 272179219306778901, 207974891850547994, 463414006872816076, 11227325073888635, 56136925292941898, 280684926388208213, 250503427257694554, 99595931605126259, 497979957949130018, 184057080455458345, 343824949897367831, 566203544803492644, 525175014727271475, 320032364346165630, 447240617047481639, 506821128250639067, 228262931963003590, 564854207435094056, 518428327885278535, 286298930136200930, 278573445997658139, 239946025304944184, 46808921841374409, 234044909130370768, 17303340968507329, 86517004766035368, 432585323753675563, 433544661781608687, 438341351921274307, 462324802619602407, 5781303807820290, 28906818962600173, 144534394736499588, 146211521302574046, 154597154132946336, 196525318284807786, 406166139044115036, 301448738233806052, 354322486485683749, 42230475441649617, 211152677131746808, 479302933278810146, 90671957103858985, 453360085442793648, 537418470227199112, 381249641845803815, 176866252242249947, 307870808831325841, 386432839473282694, 202782240379644342, 437450749518297816, 457871790604719952, 559976996036830632, 494042270893961415, 164368645179615330, 245382773518152756}
    };

    // The code used to generate the zobrist keys is below

//    linear_congruential_engine<unsigned long,5,299923498723,576460752303422617ULL> ZOBRIST;
//    unsigned long SEED = 8097652132746016;
//
//    for (int j = 0; j < 12; j++) {
//    ZOBRIST.seed(SEED + j * 439837465983746598);
//    cout << "{";
//    for (int i = 0; i < 64; i++) {
//    cout << ZOBRIST.operator()();
//    if (i != 63)
//    cout << ", ";
//}
//cout << "}" << endl;
//}

    constexpr const static unsigned long IS_IT_WHITE_TO_MOVE_CODE = 1;
    constexpr const static unsigned long WHITE_CAN_CASTLE_SHORT_CODE = 2;
    constexpr const static unsigned long BLACK_CAN_CASTLE_SHORT_CODE = 4;
    constexpr const static unsigned long WHITE_CAN_CASTLE_LONG_CODE = 8;
    constexpr const static unsigned long BLACK_CAN_CASTLE_LONG_CODE = 16;

    const static unsigned long WHICH_PAWN_MOVED_TWO_SQUARES_CODES[256] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310, 28376491298310};
}

#endif //AMETHYST_CHESS_ZOBRISTHASHING_H
