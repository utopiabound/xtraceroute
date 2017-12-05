/* $Id: countries.c,v 1.4 2003/02/22 23:12:40 d3august Exp $
*/
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-1998  Björn Augustsson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*       Country suffix database for xtraceroute       */

#include "xt.h"

/*   The .gov suffix defaults to Washington.  */
/*   The .mil suffix currently doesn't have a default. I'm not sure what
     to do. Pentagon? Groom lake?  */
/*   The .edu suffix currently doesn't have a default.  */

const countrycode countries[] = 
{
 {  38.53, -77.02, "GOV", N_("US Government")},

 {  -7.93, -14.37,  "AC", N_("Ascension Island")},
 {  42.50,   1.53,  "AD", N_("Principalty of Andorra")},
 {  24.47,  54.37,  "AE", N_("United Arab Emirates")},
 {  34.35,  69.12,  "AF", N_("Afghanistan")},
 {  17.00, -62.00,  "AG", N_("Antigua and Barbuda")},	/* approximative */
 {  18.00, -63.00,  "AI", N_("Anguilla")},		/* approximative */
 {  41.33,  19.82,  "AL", N_("Albania")},
 {  40.00,  44.00,  "AM", N_("Armenia")},
 {   0.00,   0.00,  "AN", N_("Netherlands Antilles")},
 {  -8.83,  13.25,  "AO", N_("Angola")},
 { -90.00,   0.00,  "AQ", N_("Antarctica")},
 { -34.35, -58.29,  "AR", N_("Argentina")},
 { -14.50,-171.00,  "AS", N_("American Samoa")},	/* approximative */
 {  48.15,  16.22,  "AT", N_("Austria")},
 { -27.00, 133.00,  "AU", N_("Australia")},
 {  12.50, -70.00,  "AW", N_("Aruba")},			/* approximative */
 {  42.00,  45.00,  "AZ", N_("Azerbaijan")},
 {  43.87,  18.43,  "BA", N_("Bosnia and Herzegowina")},
 {  13.00, -59.00,  "BB", N_("Barbados")},		/* approximative */
 {  22.21,  91.50,  "BD", N_("Bangladesh")},
 {  50.83,   4.33,  "BE", N_("Belgium")},
 {  12.33,  -1.67,  "BF", N_("Burkina Faso")},
 {  42.42,  23.20,  "BG", N_("Bulgaria")},
 {  26.02,  50.55,  "BH", N_("Bahrain")},
 {  -3.37,  29.32,  "BI", N_("Burundi")},
 {   6.50,   2.78,  "BJ", N_("Benin")},
 {  33.22, -64.41,  "BM", N_("Bermuda")},
 {   5.00, 114.98,  "BN", N_("Brunei Darussalam")},
 { -16.30, -68.09,  "BO", N_("Bolivia")},
 { -15.52, -47.55,  "BR", N_("Brazil")},
 {  25.05, -77.21,  "BS", N_("Bahamas")},
 {  28.00,  90.00,  "BT", N_("Bhutan")},		/* approximative */
 { -55.00,   2.00,  "BV", N_("Bouvet Island")},		/* approximative */
 { -24.75,  25.92,  "BW", N_("Botswana")},
 {  53.54,  27.33,  "BY", N_("Belarus")},
 {  17.25, -88.78,  "BZ", N_("Belize")},
 {  45.4166, -75.7, "CA", N_("Canada")},
 {   5.50, -87.00,  "CC", N_("Cocos Islands")},		/* approximative */
 {  -4.33,  15.30,  "CD", N_("Congo, Democratic Republic of the")},
 {   4.38,  18.61,  "CF", N_("Central African Republic")},
 {   4.15,  15.15,  "CG", N_("Congo")},
 {  47.23,   8.33,  "CH", N_("Switzerland")},
 {   5.19,   4.01,  "CI", N_("Cote d'Ivoire")},
 { -15.00,-160.00,  "CK", N_("Cook Islands")},		/* approximative */
 { -33.27, -70.42,  "CL", N_("Chile")},
 {   3.85,  11.52,  "CM", N_("Cameroon")},
 {  39.917, 116.417, "CN", N_("China")},
 {   4.36, -74.05,  "CO", N_("Colombia")},
 {   9.95, -84.08,  "CR", N_("Costa Rica")},
 {  23.08, -82.21,  "CU", N_("Cuba")},
 {  14.92, -23.50,  "CV", N_("Cape Verde")},
 { -10.00, 105.00,  "CX", N_("Christmas Island")},	/* approximative */
 {  35.00,  33.00,  "CY", N_("Cyprus")},
 {  50.05,  14.25,  "CZ", N_("Czech Republic")},
 {  52.53,  13.42,  "DE", N_("Germany")},
 {  11.60,  43.15,  "DJ", N_("Djibouti")},
 {  55.41,  12.33,  "DK", N_("Denmark")},
 {  15.00, -62.00,  "DM", N_("Dominica")},		/* approximative */
 {  18.29, -69.54,  "DO", N_("Dominican Republic")},
 {  36.46,  30.03,  "DZ", N_("Algeria")},
 {  -0.13, -78.32,  "EC", N_("Ecuador")},
 {  58.00,  22.00,  "EE", N_("Estonia")},
 {  29.52,  31.20,  "EG", N_("Egypt")},
 {  25.00, -15.00,  "EH", N_("Western Sahara")},	/* approximative */
 {  15.00,  40.00,  "ER", N_("Eritrea")},		/* approximative */
 {  40.25,  -3.41,  "ES", N_("Spain")},
 {  15.02,  38.55,  "ET", N_("Ethiopia")},
 {  60.13,  25.00,  "FI", N_("Finland" )},
 { -17.00, 178.00,  "FJ", N_("Fiji")},			/* approximative */
 { -51.50, -60.00,  "FK", N_("Falkland Islands")},	/* approximative */
 {   5.00, 150.00,  "FM", N_("Micronesia")},		/* approximative */
 {  62.50,  -7.00,  "FO", N_("Faeroe Islands")},	/* approximative */
 {  48.87,   2.33,  "FR", N_("France")},
 {   0.38,   9.42,  "GA", N_("Gabon")},
 {  12.00, -62.00,  "GD", N_("Grenada")},		/* approximative */
 {  41.43,  44.48,  "GE", N_("Georgia")},
 {   4.56, -52.27,  "GF", N_("French Guiana")},
 {  49.45,  -2.53,  "GG", N_("Guernsey")},
 {   5.33,  -0.12,  "GH", N_("Ghana")},
 {  36.09,  -5.22,  "GI", N_("Gibraltar")},
 {  61.11, -45.25,  "GL", N_("Greenland")},
 {  13.47, -16.65,  "GM", N_("Gambia")},
 {   9.52, -13.72,  "GN", N_("Guinea")},
 {  16.00, -62.50,  "GP", N_("Guadaloupe")},		/* approximative */
 {   1.50, -10.00,  "GQ", N_("Equatorial Guinea")},	/* approximative */
 {   38.0,   23.73, "GR", N_("Greece" )},
 { -59.00, -28.00,  "GS", N_("S. Georgia & Sandwich")},	/* approximative */
 {  14.06, -90.31,  "GT", N_("Guatemala")},
 {  14.00, 145.00,  "GU", N_("Guam")},			/* approximative */
 {  11.86, -15.65,  "GW", N_("Guinea-Bissau")},
 {   6.50, -58.12,  "GY", N_("Guyana")},
 {  22.18, 114.10,  "HK", N_("Hong Kong")},
 { -54.00,  75.00,  "HM", N_("Heard and McDonald Islands")}, /* approximative*/
 {  14.06, -87.13,  "HN", N_("Honduras")},
 {  45.00,  16.00,  "HR", N_("Hrvatska (Croatia)")},
 {  18.33, -72.20,  "HT", N_("Haiti, Republic of")},
 {  47.31,  19.02,  "HU", N_("Hungary")},
 {  -6.11, 106.50,  "ID", N_("Indonesia")},
 {  53.22,  -6.21,  "IE", N_("Ireland")},
 {  31.47,  35.13,  "IL", N_("Israel")},
 {  54.27,  -4.50,  "IM", N_("Isle of Man")},
 {  28.35,  77.12,  "IN", N_("India")},
 {  -5.00,  75.00,  "IO", N_("British Indian Ocean Territory")}, /* approx. */
 {  33.20,  44.24,  "IQ", N_("Iraq")},
 {  35.41,  51.25,  "IR", N_("Iran")},
 {  64.15, -21.97,  "IS", N_("Iceland" )},
 {  41.48,  12.36,  "IT", N_("Italy")},
 {  49.20,  -2.62,  "JE", N_("Jersey")},
 {  18.00, -76.75,  "JM", N_("Jamaica")},
 {  31.57,  35.57,  "JO", N_("Jordan, Hashemite Kingdom of")},
 {  35.75, 139.75,  "JP", N_("Japan")},
 {  -1.16,  36.48,  "KE", N_("Kenya, Republic of")},
 {  42.00,  74.00,  "KG", N_("Kyrgyz Republic")},
 {  11.33, 104.51,  "KH", N_("Cambodia, Kingdom of")},
 {  -5.00,-175.00,  "KI", N_("Kiribati")},		/* approximative */
 { -11.67,  43.27,  "KM", N_("Comoros")},
 {  17.00, -63.00,  "KN", N_("Saint Kitts and Nevis")}, /* approximative */
 {  39.02, 125.41,  "KP", N_("Korea (North)")},
 {  37.34, 126.58,  "KR", N_("Korea (South)")},
 {  29.00,  48.00,  "KW", N_("Kuwait")},
 {  19.30, -81.38,  "KY", N_("Cayman Islands")},
 {  43.14,  76.53,  "KZ", N_("Kazakhstan")},
 {  17.00, 104.00,  "LA", N_("Laos")},
 {  33.54,  35.28,  "LB", N_("Lebanon")},
 {  14.00, -61.00,  "LC", N_("Saint Lucia")},		/* approximative */
 {  47.15,   9.52,  "LI", N_("Liechtenstein")},
 {   6.54,  79.52,  "LK", N_("Sri Lanka")},
 {   6.18, -10.48,  "LR", N_("Liberia, Republic of")},
 { -29.32,  27.48,  "LS", N_("Lesotho, Kingdom of")},
 {  55.00,  25.00,  "LT", N_("Lithuania")},
 {  49.60,   6.15,  "LU", N_("Luxembourg, Grand Duchy of")},
 {  57.00,  24.00,  "LV", N_("Latvia")},
 {  32.06,  20.40,  "LY", N_("Libya")},
 {  33.35,  -7.39,  "MA", N_("Morocco, Kingdom of")},
 {  43.75,   7.42,  "MC", N_("Monaco, Principality of")},
 {  46.50,  29.50,  "MD", N_("Moldova, Republic of")},	/* approximative */
 { -18.55,  47.33,  "MG", N_("Madagascar, Republic of")},
 {  10.00, 170.00,  "MH", N_("Marshall Islands")},	/* approximative */
 {  42.00,  21.47,  "MK", N_("Macedonia")},
 {  13.47,  -7.98,  "ML", N_("Mali")},
 {  16.47,  96.09,  "MM", N_("Myanmar")},
 {  47.00, 107.00,  "MN", N_("Mongolia")},
 {  22.00, 113.00,  "MO", N_("Macau")},
 {  15.00, 145.00,  "MP", N_("Northern Mariana Islands")}, /* approximative */
 {  14.37, -61.05,  "MQ", N_("Martinique")},
 {  18.15, -15.97,  "MR", N_("Mauritania")},
 {  16.50, -62.50,  "MS", N_("Montserrat")},		/* approximative */
 {  14.31,  35.48,  "MT", N_("Malta, Republic of")},
 { -20.17,  57.50,  "MU", N_("Mauritius")},
 {   4.17,  73.50,  "MV", N_("Maldives, Republic of")},
 { -18.97,  33.82,  "MW", N_("Malawi, Republic of")},
 {  19.40, -99.15,  "MX", N_("Mexico")},
 {   3.07, 101.42,  "MY", N_("Malaysia")},
 { -25.97,  32.58,  "MZ", N_("Mozambique")},
 { -22.20,  17.10,  "NA", N_("Namibia")},		
 { -22.50, 167.00,  "NC", N_("New Caledonia")},		/* approximative */
 {  13.53,   2.08,  "NE", N_("Niger, Republic of the")},
 { -29.00, 168.00,  "NF", N_("Norfolk Island")},	/* approximative */
 {   6.27,   3.24,  "NG", N_("Nigeria, Federal Republic of")},
 {  12.10,  86.15,  "NI", N_("Nicaragua, Republic of")},
 {  52.35,   4.90,  "NL", N_("Netherlands, Kingdom of the")},
 {  59.93,  10.75,  "NO", N_("Kingdom of Norway"  )},
 {  27.42,  85.12,  "NP", N_("Nepal, Kingdom of")},
 {  -0.50, 166.50,  "NR", N_("Nauru, Republic of")},	/* approximative */
 { -19.00,-169.00,  "NU", N_("Niue")},
 { -41.17, 174.46,  "NZ", N_("New Zealand")},
 {  23.38,  58.50,  "OM", N_("Oman, Sultanate of")},
 {   8.58, -79.33,  "PA", N_("Panama, Republic of")},
 { -12.05, -77.03,  "PE", N_("Peru, Republic of")},
 { -17.50,-149.50,  "PF", N_("French Polynesia")},	/* approximative */
 {  -9.29, 147.09,  "PG", N_("Papua New Guinea")},
 {  14.35, 120.59,  "PH", N_("Philippines, Republic of the")},
 {  24.48,  66.59,  "PK", N_("Pakistan, Islamic Republic of")},
 {  52.13,  21.02,  "PL", N_("Poland")},
 {  47.00, -56.50,  "PM", N_("St. Pierre et Miquelon")}, /* approximative */
 { -24.00,-130.00,  "PN", N_("Pitcairn Island")},	/* approximative */
 {  18.29, -66.07,  "PR", N_("Puerto Rico")},
 {  31.78,  35.22,  "PS", N_("Palestinian Territories")},
 {  36.43,  -9.06,  "PT", N_("Portugal, Portuguese Republic")},
 {   8.00, 135.00,  "PW", N_("Palau")},			/* approximative */
 { -25.17, -57.30,  "PY", N_("Paraguay, Republic of")},
 {  25.03,  51.47,  "QA", N_("Qatar, State of")},
 { -22.00,  55.50,  "RE", N_("Reunion")},		/* approximative */
 {  44.43,  26.10,  "RO", N_("Romania")},
 {  55.46,  37.40,  "RU", N_("Russian Federation")},
 {  -1.93,  30.07,  "RW", N_("Rwanda, Rwandese Republic")},
 {  24.39,  46.42,  "SA", N_("Saudi Arabia, Kingdom of")},
 {  -9.50, 160.00,  "SB", N_("Solomon Islands")},	/* approximative */
 {  -4.63,  55.47,  "SC", N_("Seychelles, Republic of")},
 {  15.37,  32.33,  "SD", N_("Sudan")},
 {  59.33,  18.08,  "SE", N_("Kingdom of Sweden")},
 {   1.18, 103.50,  "SG", N_("Singapore, Republic of")},
 { -15.92,  18.17,  "SH", N_("St. Helena")},
 {  46.07,  14.50,  "SI", N_("Slovenia")},
 {  72.00,  -9.00,  "SJ", N_("Svalbard & Jan Mayen Islands")}, /* approx. */
 {  48.17,  17.17,  "SK", N_("Slovakia")},
 {   8.50, -13.28,  "SL", N_("Sierra Leone, Republic of")},
 {  43.92,  12.47,  "SM", N_("San Marino, Republic of")},
 {  14.42, -17.29,  "SN", N_("Senegal, Republic of")},
 {   2.02,  49.19,  "SO", N_("Somalia")},
 {   5.49, -55.09,  "SR", N_("Suriname, Republic of")},
 {   0.33,   6.73,  "ST", N_("Sao Tome and Principe")},
 {  13.42, -89.13,  "SV", N_("El Salvador, Republic of")},
 {  33.30,  36.20,  "SY", N_("Syrian Arab Republic")},
 { -26.33,  31.13,  "SZ", N_("Swaziland, Kingdom of")},
 {  21.50, -72.00,  "TC", N_("Turks and Caicos Islands")}, /* approximative */
 {  12.17,  14.98,  "TD", N_("Chad, Republic of")},
 {   0.00,   0.00,  "TF", N_("French Southern Territories")},
 {   6.17,   1.35,  "TG", N_("Togo, Togolese Republic")},
 {  13.44, 100.30,  "TH", N_("Thailand, Kingdom of")},
 {  38.00,  69.00,  "TJ", N_("Tajikistan")},
 {  -8.00,-172.00,  "TK", N_("Tokelau")},		/* approximative */
 {  37.00,  58.00,  "TM", N_("Turkmenistan")},
 {  36.47,  10.12,  "TN", N_("Tunisia, Republic of")},
 { -22.00,-175.00,  "TO", N_("Tonga, Kingdom of")},	/* approximative */
 {  -9.00, 125.00,  "TP", N_("East Timor")},		/* approximative */
 {  40.58,  28.50,  "TR", N_("Turkey, Republic of")},
 {  10.40, -61.31,  "TT", N_("Trinidad and Tobago")},
 {  -8.00, 178.00,  "TV", N_("Tuvalu")},		/* approximative */
 {  25.02, 121.31,  "TW", N_("Taiwan")},
 {  -6.50,  39.18,  "TZ", N_("Tanzania, United Republic of")},
 {  50.27,  30.30,  "UA", N_("Ukraine")},
 {   0.33,  32.58,  "UG", N_("Uganda, Republic of")},
 {  51.50,   0.00,  "UK", N_("United Kingdom" )},
 {   0.00,   0.00,  "UM", N_("US. Minor Outlying Islands")},
 {  38.53, -77.02,  "US", N_("United States of America")},
 { -34.51, -56.13,  "UY", N_("Uruguay, Eastern Republic of")},
 {  41.20,  69.18,  "UZ", N_("Uzbekistan")},
 {  41.48,  12.36,  "VA", N_("Vatican City State")},
 {  13.50, -61.50,  "VC", N_("St Vincent & the Grenadines")}, /* approx. */
 {  10.30, -66.56,  "VE", N_("Venezuela, Republic of")},
 {  18.50, -64.00,  "VG", N_("British Virgin Islands")}, /* approximative */
 {  18.30, -65.00,  "VI", N_("US Virgin Islands")},	/* approximative */
 {  21.02, 105.52,  "VN", N_("Viet Nam")},
 { -17.00, 169.00,  "VU", N_("Vanuatu")},		/* approximative */
 { -14.00,-178.00,  "WF", N_("Wallis and Futuna Islands")}, /* approx. */
 { -14.00,-173.00,  "WS", N_("Western Samoa")},		/* approximative */
 {  13.00,  45.00,  "YE", N_("Yemen")},			/* approximative */
 { -12.00,  44.00,  "YT", N_("Mayotte")},		/* approximative */
 {  44.48,  20.28,  "YU", N_("Montenegro and Serbia")},
 { -33.93,  18.47,  "ZA", N_("Republic of South Africa" )},
 { -15.46,  28.26,  "ZM", N_("Zambia, Republic of")},
 {  -4.20,  15.18,  "ZR", N_("Zaire, Republic of")},
 { -17.83,  31.05,  "ZW", N_("Zimbabwe")}
};

const int n_countries  = sizeof(countries) / sizeof(countrycode);

