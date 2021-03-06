#include "../include/div_RTL.hh"
using namespace sc_dt;


void divModule::div_proc_dp() {
	NEXT_STATE = STATE;

	switch (STATE) {
	case Reset_ST:
		NEXT_STATE = ST_0;
		break;

	case ST_0:
		if (start.read() == 1) {
			NEXT_STATE = ST_1;
		}
		break;

	case ST_1:
		NEXT_STATE = ST_2;
		break;

	case ST_2:
		if (flagExt == 0) {
			NEXT_STATE = ST_3;
		} else {
			NEXT_STATE = ST_5;
		}
		break;

	case ST_3:
		if (flagInt == 0) {
			NEXT_STATE = ST_4;
		} else {
			NEXT_STATE = ST_2;
		}
		break;

	case ST_4:
		NEXT_STATE = ST_3;
		break;

	case ST_5:
		NEXT_STATE = ST_6;
		break;

	case ST_6: // eccezione
		NEXT_STATE = ST_7;
		break;

	case ST_7:
		NEXT_STATE = ST_8;
		break;

	case ST_8:
		NEXT_STATE = Reset_ST;
		break;
	}
}

void divModule::div_proc_fsm() {

	static sc_lv<64> tmp_value1; // operandi
	static sc_lv<64> tmp_value2;
	static sc_uint<11> y1;
	static sc_uint<11> y2;
	static sc_lv<11> e1;
	static sc_lv<11> e2;
	static sc_uint<11> esponente;
	static int p; // per normalizzazione in st_6
	static int pos; // contatore ciclo esterno
	static int pos2; // contatore ciclo interno
	static sc_lv<106> prodotto; // prodotto delle mantisse
	static sc_lv<53> mantissa2;
	static sc_lv<53> mantissa1;
	static sc_lv<106> tmp; // vettore temporaneo doppio ciclo
	static sc_lv<64> finale;
	static sc_lv<11> espo;
	static sc_lv<106> ris; // somme parziali
	static sc_logic c; // carry somme parziali


	if (rst.read() == 1) {
		STATE = Reset_ST;
	}

	else if (clk.read() == 1) {

		STATE = NEXT_STATE;

		switch (STATE) {
		case Reset_ST:
			done.write(sc_logic('0'));
			esponente = 0;
			pos = -1;
			y1 = 0;
			y2 = 0;
			e1 = "0";
			e2 = "0";
			flagInt = 0;
			flagExt = 0;
			flagTmp = 0;
			prodotto = "0";
			espo = "0";
			ris = "0";
			c = sc_logic('0');
			mantissa1 = "0";
			mantissa2 = "0";
			tmp = "0";
			finale = "0";
			flagInt = 0;
			flagExt = 0;
			flagTmp = 0;
			break;

		case ST_0:
			tmp_value1 = op1in.read();
			tmp_value2 = op2in.read();
			cout << "rtl"<<tmp_value1 << "\t"<<tmp_value2 << endl;
			break;

		case ST_1:
			e1.range(10, 0) = tmp_value1.range(62, 52);
			e2.range(10, 0) = tmp_value2.range(62, 52);
			y1 = static_cast<sc_uint<11> >(e1);
			y2 = static_cast<sc_uint<11> >(e2);
			esponente = y1 + y2 - 1023;

			mantissa2 = tmp_value2.range(51, 0);
			mantissa1 = tmp_value1.range(51, 0);
			mantissa2[52] = 1;
			mantissa1[52] = 1;
			tmp = mantissa1;
			pos = -1;

			break;

		case ST_2:

			pos2 = 0;
			pos++;
			flagTmp = 1;
			if (pos < 53) {
				flagInt = 0;
				if (pos >= 1) {
					tmp = mantissa1;
				}
				if (mantissa2[pos] != 0) {
					tmp = tmp << pos;
					flagTmp = 0;
				}

			} else {
				flagExt = 1;
			}

			break;

		case ST_3:

			if (pos2 >= 106) {
				flagInt = 1;
				c = sc_logic('0');
				prodotto = ris;
			}
			break;

		case ST_4:

			if (flagTmp == 0) {

				if (prodotto[pos2] == 0 && tmp[pos2] == 0) {
					if (c == 0)
						ris[pos2] = sc_logic('0');
					else {
						ris[pos2] = sc_logic('1');
						c = sc_logic('0');
					}
				}
				if (prodotto[pos2] == 1 && tmp[pos2] == 0) {
					if (c == 0)
						ris[pos2] = sc_logic('1');
					else
						ris[pos2] = sc_logic('0');
				}
				if (prodotto[pos2] == 0 && tmp[pos2] == 1) {
					if (c == 0)
						ris[pos2] = sc_logic('1');
					else
						ris[pos2] = sc_logic('0');
				}
				if (prodotto[pos2] == 1 && tmp[pos2] == 1) {
					if (c == 0) {
						ris[pos2] = sc_logic('0');
						c = sc_logic('1');
					} else
						ris[pos2] = sc_logic('1');
				}
				if (pos2 == 54 + pos) {
					(c == 1) ?
							ris[pos2] = sc_logic('1') :
							ris[pos2] = sc_logic('0');
				}
				pos2++;
			} else {
				pos2++;
			}

			break;

		case ST_5:
			p = 0;
			if (prodotto[105] == sc_logic('1')) {
				p += pow(2, 1);
			}
			if (prodotto[104] == sc_logic('1')) {
				p += pow(2, 0);
			}
			if (p >= 2) {
				esponente++; //esponente
				prodotto = prodotto >> 1; // shift
			}

			break;
		case ST_6:
			if (esponente == 2047) {
				/*if (parteDecimale == 0 && dec <= 2) {
					cout << "Error : Infinity" << endl;
					exit(1);
				}*/
				if (prodotto == 0) {
					cout << "Error : Infinity" << endl;
					exit(1);
				}
				if (prodotto != 0) {
					cout << "Error : NaN" << endl;
					exit(1);
				}
			}
			break;

		case ST_7:
			espo = static_cast<sc_lv<11> >(esponente);

			finale.range(62, 52) = espo(10, 0);
			finale.range(51, 0) = prodotto(103, 52);

			//controllo del segno

			(tmp_value1[63] != tmp_value2[63]) ?finale[63] = sc_logic('1') : finale[63] = sc_logic('0');

			cout << logicVectorToDouble(finale) << endl;
			break;

		case ST_8:
			done.write(sc_logic('1'));
			result.write(finale);
			break;
		}
	}
}

