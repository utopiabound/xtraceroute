/* $Id: db.c,v 1.2 2003/02/22 23:17:09 d3august Exp $
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

/*    Database of locations/names for xtraceroute.   */

/*    Format is:
  {   lat, lon, ip, name, info  }

      Positive Latitudes are on the northern hemisphere.
      Positive Longitudes are on the eastern hemisphere.
*/ 

#include "xt.h"
#include <stdlib.h>

dbentry int_dbentries[] =
{
  {   59.33,   18.08, "", "nordu.net",   "Stockholm, Sweden"    },
  {   57.5,    12.0,  "", "gbg.sunet.se","G|teborg, Sweden"     },
  {   57.5,    12.0,  "", "chalmers.se", "G|teborg, Sweden"     },
  {   60.5,    24.5,  "", "funet.fi",    "Finland"              },
  {   38.53,  -77.02, "", "-dc-",        "Washington, DC"       },
  {   39.55,  -75.07, "", "-pen-",       "Pennsauken, NJ"       },
  {   51.5,     0.0,  "", "-uk-",        "London, UK"           },
  {   37.52, -122.16, "", "berkeley",    "Berkeley, CA"         },
  {   37.26, -122.08, "", "paloalto",    "Palo Alto, CA"        },
  {   42.21,  -71.06, "", "cambridge",   "Cambridge, MA"        },
  {   42.21,  -71.06, "", "mit.edu",     "Cambridge, MA"        },
  {   40.42,  -74.00, "", "newyork",     "New York, NY"         },
  {   40.26,  -79.59, "", "cmu.edu",     "Pittsburgh, PA"       },
  {  -33.93,   18.47, "", "iafrica.net", "South Africa"         },
  {   52.35,    4.90, "", "Amsterdam",   "Amsterdam, Holland"   },
  {   48.58,    7.75, "", "strasbourg",  "Strasbourg, France"    },
  {   48.58,    7.75, "", "u-strasbg.fr","Strasbourg University, France"},
  {   47.9,    -1.36, "", "nantes",      "Nantes,France"        },
  {   44.49,   -0.42, "", "bordeaux",    "Bordeaux,France"      },
  {   43.26,    5.12, "", "marseille",   "Marseille,France"     },
  {   45.39,    4.54, "", "lyon",        "Lyon,France"          },
  {   43.32,    1.21, "", "toulouse",    "Toulouse,France"      },
 
  /*  Here starts data (72 locations) taken from Xearth: */
  
  {  61.17,  -150.00, "",  "Anchorage", "Alaska, USA"  },
  {  38.00,  23.73, "",  "Athens", "Greece"  },
  {  33.40,  44.40, "",  "Baghdad", "Iraq"  },
  {  13.73,  100.50,"",   "Bangkok", "Thailand"  },
  {  39.92,  116.43, "",  "Beijing", "China"  },
  {  52.53,  13.42, "",  "Berlin", "Germany"  },
  {  32.30,  -64.70,"",   "Bermuda", "Bermuda"  },
  {  42.33,  -71.08, "",  "Boston", "Massachusetts, USA"  },
  {  -15.80,  -47.90, "",  "Brasilia", "Brazil"  },
  {  -4.20,  15.30,"",   "Brazzaville", "Congo"  },
  {  -34.67,  -58.50, "",  "Buenos", "Aires"  },
  {  31.05,  31.25, "",  "Cairo", "Egypt"  },
  {  22.50,  88.30, "",  "Calcutta", "India"  },
  {  -33.93,  18.47, "",  "CapeTown", "South Africa"  },
  {  33.60,  -7.60, "",  "Casablanca", "Morocco"  },
  {  41.83,  -87.75, "",  "Chicago", "Illinois, USA"  },
  {  32.78,  -96.80, "",  "Dallas", "Texas, USA"  },
  {  28.63,  77.20, "",  "NewDelhi", "India"  },
  {  39.75,  -105.00, "",  "Denver", "Colorado, USA"  },
  {  24.23,  55.28, "",  "Dubai", "UAE"  },
  {  -27.10,  -109.40,"",   "EasterIsland", "Easter Island"  },
  {  -18.00,  178.10, "",  "Fiji", "Fiji"  },
  {  13.50,  144.80, "",  "Guam", "Guam"  },
  {  60.13,  25.00, "",  "Helsinki", "Finland"  },
  {  22.20,  114.10, "",  "HongKong", "Hong Kong"  },
  {  21.32,  -157.83,"",   "Honolulu", "Hawaii, USA"  },
  {  52.20,  104.30, "",  "Irkutsk", "Irkutsk, Russia"  },
  {  41.00,  29.00, "",  "Istanbul", "Turkey"  },
  {  -6.13,  106.75, "",  "Jakarta", "Indonesia"  },
  {  31.80,  35.20,  "", "Jerusalem", "Israel"  },
  {  34.50,  69.20,"",   "Kabul", "Afghanistan"  },
  {  27.70,  85.30,"",   "Kathmandu", "Nepal"  },
  {  50.40,  30.50,"",   "Kiev", "Ukraine"  },
  {  3.13,  101.70, "",  "Kuala Lumpur", "Malaysia"  },
  {  6.45,  3.47, "",  "Lagos", "Nigeria"  },
  {  -12.10,  -77.05,"",   "Lima", "Peru"  },
  {  51.50,  -0.17, "",  "London", "UK"  },
  {  40.42,  -3.72, "",  "Madrid", "Spain"  },
  {  14.60,  121.00, "",  "Manila", "The Phillipines"  },
  {  21.50,  39.80, "",  "Mecca", "Saudi Arabia"  },
  {  19.40,  -99.10,"",   "Mexico City", "Mexico"  },
  {  25.80,  -80.20, "",  "Miami", "Florida, USA"  },
  {  6.20,  -10.80, "",  "Monrovia", "Liberia"  },
  {  45.50,  -73.50, "",  "Montreal", "Quebec, Canada"  },
  {  55.75,  37.70, "",  "Moscow", "Russia"  },
  {  -1.28,  36.83, "",  "Nairobi", "Kenya"  },
  {  59.93,  10.75, "",  "Oslo", "Norway"  },
  {  48.87,  2.33, "",   "Paris", "France"  },
  {  -32.00,  115.90,"",   "Perth", "Australia"  },
  {  45.50,  -122.50,"",   "Portland", "Oregon, USA"  },
  {  -0.20,  -78.50, "",  "Quito", "Ecuador"  },
  {  64.15,  -21.97, "",  "Reykjavik", "Iceland"  },
  {  41.88,  12.50,"",   "Rome", "Italy"  },
  {  37.75,  -122.45, "",  "San Francisco", "Califonia, USA"  },
  {  9.98,  -84.07, "",  "San Jose", "Costa Rica"  },
  {  18.50,  -66.10, "",  "San Juan", "Puerto Rico"  },
  {  -33.50,  -70.70, "",  "Santiago", "Chile"  },
  {  1.20,  103.90, "",  "Singapore", "Singapore"  },
  {  42.67,  23.30, "",  "Sofia", "Bulgaria"  },
  {  59.33,  18.08, "",  "Stockholm", "Sweden"  },
  {  -33.92,  151.17, "",  "Sydney", "Australia"  },
  {  -17.60,  -149.50,"",   "Tahiti", "Tahiti"  },
  {  16.80,  -3.00, "",  "Timbuktu", "Mali"  },
  {  35.67,  139.75, "",  "Tokyo", "Japan"  },
  {  43.70,  -79.42,"",   "Toronto", "Ontario, Canada"  },
  {  32.90,  13.20, "",  "Tripoli", "Libya"  },
  {  47.90,  106.90, "",  "Ulan", "Bator"  },
  {  49.22,  -123.10, "",  "Vancouver", "B.C., Canada"  },
  {  48.22,  16.37, "",  "Wien", "Austria"  },
  {  38.90,  -77.00, "",  "Washington", "USA"  },
  {  -41.28,  174.78, "",  "Wellington", "New Zealand"  },
  {  62.50,  -114.30, "",  "Yellowknife", "N.T., Canada"  },

/* Other fun ones */

  {  -78,  165, "",  "McMurdo", "Antarctica"  },
};

database *init_internal_db(void)
{
  database *tmp;

  tmp = (database *)malloc(sizeof(database));
  tmp->entries = int_dbentries;
  tmp->n_entries = sizeof(int_dbentries) / sizeof(dbentry);
  
  return tmp;
}
