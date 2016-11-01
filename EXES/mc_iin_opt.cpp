/*
./qp_iin_opt_matrix_vers.exe parms.inp nodes.inp edges.inp <IIN_REF>
4th argument is optional. 

This program uses the PPIN structure and a fitness function to try and define an optimal IIN.

The fitness function used to select between IINs depends on several topological features:
- The clustering coefficients of each node. (Penalizes high clustering)
- The (modified) grid coefficients of each node (Favors squares, penalizes unclosed chains)
- The number of interfaces (penalizes too many interfaces in the network)
- The number of edges (penalizes addition of extra edges)
- (Optional) Self-looped nodes with additional edges are penalized

Each iteration, the IIN is mutated in some way. Moves that increase the fitness are accepted, whereas moves that decrease fitness can be accepted according to a Boltzmann weight probability. This prevents being stuck in a local minima. Also, a second probability factor is applied to the probabilities in order to maintain detailed balance, ie make sure the mutations are not biased the network.
*/

#include <fstream>
#include <iostream>
#include <ctime>
#include <cmath>
#include <stdlib.h>
#include <sys/time.h>

#include "md_timer.h"
#include "rand_gsl.h"
#include "pro_classes.h"
#include "constrainParms.h"
#include "metrics_class.h"
#include "matmultiply.h"
//#include "qp_solvers.h"
#include "read_iin_parms.h"
#include "init_iin_net.h"
#include "modify_network_only.h"
#include "read_yeast_conc.h"
#include "read_proinput.h"
#include "network_metrics2.h"

using namespace std;

struct MD_Timer totaltime;
struct MD_Timer inittime;



int main(int argc, char *argv[])
{
  
  timeval tim;
  double t1;
  int seed;
  gettimeofday(&tim, 0);
  t1=tim.tv_sec+tim.tv_usec;
  //  seed=1452601;//
  seed=static_cast<unsigned>(t1);
  
  /*Here we want every processor to be different*/
  srand_gsl(seed);
  
  int i, j;

  ifstream parmfile(argv[1]);
  constrainParms plist;
  
  read_iin_parms(parmfile, plist);

  cout <<"Nentropy averages: "<<plist.Nentavg<<endl;
  
  int nwhole=plist.nwhole; //number of proteins

  
  string *genid=new string[nwhole];
  double *abund=new double[nwhole];
  
  int t=0;
  
  ifstream expfile(argv[2]);
  for(i=0;i<nwhole;i++){
    expfile>>genid[i];
    abund[i]=100; //Placeholder for obsolete variable
    cout <<genid[i]<<endl;

  }

  ppidata *ppi=new ppidata[nwhole];
  
  /*read in the network of protein protein interactions*/
  int Nedge=plist.Nedge;
  ifstream edgefile(argv[3]);
  string *e1=new string[Nedge];
  string *e2=new string[Nedge];
  string ig;
  edgefile.ignore(400,'\n');//skip first line
  for(i=0;i<Nedge;i++){
    edgefile >>e1[i]>>ig>>e2[i];
        cout <<e1[i]<<' '<<e2[i]<<endl;
  }
  cout <<"read in edges "<<endl;
  double *Padj=new double[nwhole*nwhole];
  for(i=0;i<nwhole*nwhole;i++)
    Padj[i]=0;

  for(i=0;i<nwhole;i++)
    ppi[i].nppartner=0;
  
  int p;
  int p1, p2;
  int *e1num=new int[Nedge];
  int *e2num=new int[Nedge];
  t=0;
  
  for(i=0;i<Nedge;i++){
    /*its possible there is an edge in the network with a node not included */
    p1=-1;
    p2=-1;
    for(j=0;j<nwhole;j++){
      if(e1[i]==genid[j])
	p1=j;
      if(e2[i]==genid[j])
	p2=j;
    }
    if(p1==-1 || p2==-1){
      cout <<"unmatched edge: "<<i<<" skipping..." <<endl;

    }
    else{
      e1num[t]=p1;
      e2num[t]=p2;
      t++;
      /*Protein-Protein adjacency matrix*/
      Padj[p1*nwhole+p2]=1;
      Padj[p2*nwhole+p1]=1;
      if(p1==p2){
	//self interaction
	cout <<"self interaction: "<<p1<<' '<<genid[p1]<<endl;

	Padj[p2*nwhole+p1]=2;
	p=ppi[p1].nppartner;
	ppi[p1].pplist[p]=p1;
	ppi[p1].nppartner++;
      }
      else{
	p=ppi[p1].nppartner;
	ppi[p1].pplist[p]=p2;
	ppi[p1].nppartner++;
	p=ppi[p2].nppartner;
	ppi[p2].pplist[p]=p1;
	ppi[p2].nppartner++;
      }
    }
  }
  Nedge=t;
  cout <<"total protein interactions: "<<Nedge<<endl;
  /*read in data on proteins levels to constrain*/
  /*assign a unique interface to each protein*/
  int Nif=Nedge*2.0;//this is actually the maximum, it will be less if there are self interactions
  Protein *wholep=new Protein[nwhole];
  int *p_home=new int[Nif];//this reverses and tells you what protein a given interface belongs to
  t=0;
  int nint;


  /*Be careful with self interactions, for now, just also make them have a distinct interface, but it
   binds to itself, otherwise you create a chain, head to tail , head to tail*/
  
  //read in the ppidata
  double degree;
  degree=2.0*Nedge/(1.0*nwhole);
  cout <<"Number of edges: "<<Nedge<<" N proteins: "<<nwhole<<" Degree: "<<degree<<endl;
  plist.ncomplex=Nedge;
  
  int maxnif=Nedge*3;//*2;
  cout <<"Maximum number of interfaces: "<<maxnif<<endl;

  char fname[300];
 
  int **Speclist=new int*[maxnif];
  for(i=0;i<maxnif;i++)
    Speclist[i]=new int[MAXP];
  int *numpartners=new int[maxnif];

  int **templist=new int*[maxnif];
  int *tmppartners=new int[maxnif];
  for(i=0;i<maxnif;i++)
    templist[i]=new int[MAXP];
  
  
  Protein *wholetemp=new Protein[nwhole];
  int *nbor=new int[maxnif];
  /*This is a true adjacency matrix for if two interfaces happen to interact, it will be zeroed out constantly
    to use in mutation moves*/
  int *Adj=new int[maxnif*maxnif];
  

  /*edge properties*/
  int **ehome=new int *[nwhole];
  int **Epred=new int*[nwhole];
  for(i=0;i<nwhole;i++)
    Epred[i]=new int[EDIM*EDIM];
  
  for(i=0;i<nwhole;i++)
    ehome[i]=new int[nwhole];
  
  /*Ehome will not change, because full PPI does not change*/


  for(i=0;i<nwhole;i++){
    for(j=0;j<ppi[i].nppartner;j++){
      p2=ppi[i].pplist[j];
      ehome[i][p2]=j;

    }
  }
  double *Cmat=new double[Nedge*maxnif];
  double *indivconc=new double[maxnif];
  double *complexconc=new double[Nedge];

  double gstart=plist.gamma;
  double mstart=plist.mu;
  

  ofstream globalfile;
  int ninterface;
  int ngamma=1;
  int nmu=1;
  cout <<"number of gammas: "<<ngamma<<" number of mus: "<<nmu<<endl;
  double delg=0.1;
  double delm=0.1;
  
  ofstream idfile;
  ofstream matchfile;
  ifstream reffile;
  if(plist.flagsplit==1)cout <<"STARTING FROM SPLIT INTERFACE CONFIG "<<endl;
  else if(plist.flagsplit==0) cout <<"Starting from single interfaces! "<<endl;
  else if(plist.flagsplit==-1) cout <<"Using part split,part combined! "<<endl;
  else{
    reffile.open(argv[4]);
    cout <<"READING IN STARTING NETWORK FROM FILE: "<<argv[4]<<endl;
  }
  
  for(i=0;i<ngamma;i++){
    plist.gamma=gstart+i*delg;
    for(j=0;j<nmu;j++){
      plist.mu=mstart+j*delm;
      
      if(plist.flagsplit==1){
	ninterface=init_network_split(nwhole, ppi, wholep, p_home, plist,  Nedge, Speclist, numpartners, e1num, e2num);
      }
      else if(plist.flagsplit==0){
	ninterface=init_network(nwhole, ppi, wholep, p_home, plist, Nedge, Speclist, numpartners);
      }
      else if(plist.flagsplit==-1){
	ninterface=init_network_both(nwhole, ppi, wholep, p_home, plist,  Nedge, Speclist, numpartners, e1num, e2num, abund);
      }
      else{
	read_ref_ProIIN( nwhole,  wholep,  ninterface,  reffile,  numpartners,  Speclist,  p_home);
	
      }

      cout <<"Run network for gamma=: "<<plist.gamma<<" mu=: "<<plist.mu<< " kappa=: " << plist.alpha << " omega=: " << plist.omega << " start with interface number: "<<ninterface<<endl;
      
      /*The ppi has not changed
	The IIN is now in the structure iin*/
      
      cout <<"Now try modifying the network! "<<endl;
      cout <<"Current number of interfaces: "<<ninterface<<endl;
      initialize_timer(&totaltime);
      start_timer(&totaltime);
      
      modify_network_only(nwhole, ninterface, numpartners,Speclist, wholep, ppi, p_home, plist, globalfile, ehome, Epred, templist, tmppartners, wholetemp, nbor,Adj, idfile, matchfile);
      stop_timer(&totaltime);
      cout <<"time length of modifying network: "<<timer_duration(totaltime)<<endl;
    }
  }
  cout <<"END MAIN"<<endl;
  globalfile.close();
}/*End main*/



