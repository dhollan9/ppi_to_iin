#include "constrainParms.h"
#include "read_iin_parms.h"

void read_iin_parms(ifstream &parmfile, constrainParms &plist)
{
  parmfile >>plist.nwhole;
  parmfile.ignore(400,'\n');
  parmfile >>plist.Nedge;
  parmfile.ignore(400,'\n');
  parmfile >>plist.min_complex;
  parmfile.ignore(400,'\n');
  parmfile >>plist.Nconstrain;
  parmfile.ignore(400,'\n');

  
  /*Now read in network parameters, which control swaps between network
    selections*/
  parmfile >>plist.gamma;
  parmfile.ignore(400,'\n');
  parmfile >>plist.mu;
  parmfile.ignore(400,'\n');
  parmfile >>plist.Nit_net;
  parmfile.ignore(400,'\n');
  parmfile >>plist.Nequil_net;
  parmfile.ignore(400,'\n');
  parmfile >>plist.MCtemp;
  parmfile.ignore(400,'\n');
  parmfile >>plist.alpha;
  parmfile.ignore(400,'\n');
  parmfile >>plist.netwrite;
  parmfile.ignore(400,'\n');
  parmfile >>plist.phi;
  parmfile.ignore(400,'\n');
  parmfile >>plist.flagsplit;
  parmfile.ignore(400,'\n');
  parmfile >>plist.Nentavg;
  parmfile.ignore(400,'\n');
  parmfile >>plist.ascale;
  parmfile.ignore(400,'\n');
  parmfile >>plist.matrixflag;
  parmfile.ignore(400,'\n');
  parmfile >>plist.omega;
  parmfile.ignore(400,'\n');
  parmfile >>plist.zeta;
  parmfile.ignore(400,'\n');
  parmfile >>plist.PAEflag;
  parmfile.ignore(400,'\n');
  parmfile >>plist.fitnessflag;
  parmfile.ignore(400,'\n');
}

