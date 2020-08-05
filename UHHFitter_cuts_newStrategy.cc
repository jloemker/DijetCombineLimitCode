
using namespace RooFit;
using namespace RooStats ;
using namespace std;

static const Int_t NCAT = 2;
Double_t MMIN = 1050.;
Double_t MMAX = 10050.;//new range
Double_t SQRTS = 13000.;
Double_t CHANNEL = 0;
std::string filePOSTfix="";
double signalScaler=1./(10.); // assume signal cross section of 1pb (The factor 10. compensates for the factor in the interpolateUHH script)
double scaleFactorHP=1.0;// tau21 and jet mass scale factors data/MC
double scaleFactorLP=1.0;// tau21 and jet mass scale factors data/MC

bool constrainBackgroundParametersToSimulation=false; //NewStrategy. This option is useful for optimizing selection, but not adequat for analyzing data.
bool correlateBackgroundParametersBetweenCategories=true;

string directory = "./" ;
TColor *col = new TColor();


void AddSigData(RooWorkspace* w, Float_t mass, int signalsample, std::vector<string> cat_names, string cut);
void AddBkgData(RooWorkspace* w, std::vector<string> cat_names, std::string altfunc, string cut);
void SigModelFit(RooWorkspace* w, Float_t mass, TString signalname, std::vector<string> cat_names);
void SigModelFitaQGC(RooWorkspace* w, Float_t mass, TString signalname, std::vector<string> cat_names);
void MakeSigWS(RooWorkspace* w, const char* fileBaseName, TString signalname, int signalsample, std::vector<string> cat_names, TString cuts);
vector<RooFitResult*> BkgModelFit(std::string altfunc, RooWorkspace* w, Bool_t dobands, std::vector<string> cat_names,TString cuts);
void MakeBkgWS(std::string altfunc, RooWorkspace* w, const char* fileBaseName, std::vector<string> cat_names);
void SetConstantParams(const RooArgSet* params);
void MakeDataCard_1Channel(std::string altfunc,RooWorkspace* w, const char* fileBaseName, const char* fileBkgName, int iChan, TString signalname, int signalsample, std::vector<string> cat_names, double mass);
void MakePlots(RooWorkspace* w, Float_t mass, vector<RooFitResult*> fitresults, TString signalname, std::vector<string> cat_names);
void MakePrettyPlots(RooWorkspace* w, Float_t mass, vector<RooFitResult*> fitresults, TString signalname, std::vector<string> cat_names);

RooArgSet* defineVariables()
{
  // define variables of the input ntuple
  RooRealVar* mgg  = new RooRealVar("mgg13TeV","M(jet-jet)",MMIN,MMAX,"GeV");
  RooRealVar* evWeight   = new RooRealVar("evWeight","Reweightings",0,100,"");
  RooRealVar* normWeight  = new RooRealVar("normWeight","Additionnal Weight",0,10000000,"");
  RooCategory* categories = new RooCategory("categories","event category NCAT") ;
  
  categories->defineType("_invMass",0);
  categories->defineType("_invMass_afterVBFsel",1);

  RooArgSet* ntplVars = new RooArgSet(*mgg, *categories, *evWeight, *normWeight);
 
  return ntplVars;
}



void runfits(const string cuts="none",const Float_t mass=2000, int signalsample = 1, int channel = 1,std::string altfunc="", Bool_t dobands = false)
{
  
  CHANNEL = channel;
  
  std::cout << "CHANNEL == " << CHANNEL << std::endl;

  //******************************************************************//
  //  Running mode  corresponds to the following cases
  //         - full run set:
  //         - create signal and background data sets 
  //         - make and fit signal and background  models 
  //         - write signal and background workspaces in root files
  //         - write data card

  //*******************************************************************//
  

  TString signalname;
  
  if (signalsample==0){
    signalname="graviton_"+cuts;
  }
  if (signalsample==1){
    signalname="radion_"+cuts;
  }
  if (signalsample==10){
    signalname=""+cuts;
    signalScaler=1./(100.); // assume signal cross section of 1pb (The factor 100. compensates for the factor in the SignaMiniTreeProducer script)
  }
  
  std::cout<< signalname<<std::endl;
  
  TString fileBaseName("CMS_jj_"+signalname+TString::Format("_%.0f_13TeV", mass));
  vector<string> cat_names;
  cat_names.push_back("_invMass");
  cat_names.push_back("_invMass_afterVBFsel");

  TString fileBkgName("CMS_jj_bkg_VV"+altfunc+"_"+cuts+TString::Format("_%.0f_13TeV", mass));
  
  std::cout << "filebaseName : " << fileBaseName << std::endl;

  RooWorkspace* w = new RooWorkspace("w","w");
  vector<RooFitResult*> fitresults;

  w->factory(TString::Format("mgg13TeV[%.0f,%.0f]", MMIN,MMAX));
  w->factory(TString::Format("sqrtS[%.0f,%.0f,%.0f]",SQRTS,SQRTS,SQRTS));

  // Add data to the workspace

  cout << "SIGNAL SAMPLE = " << signalsample << "== "<< signalname<< endl;
 
  cout << "CREATE SIGNAL" << endl;

  AddSigData(w, mass, signalsample,cat_names, cuts);
  
  // Add the signal and background models to the workspace.
  // Inside this function you will find a discription our model.
  // Fit data with models

  cout << "FIT SIGNAL" << endl;

  if (signalsample==10)
    SigModelFitaQGC(w, mass, signalname,cat_names);
  else
    SigModelFit(w, mass, signalname,cat_names);

  // Make statistical treatment
  // Setup the limit on Higgs production

  cout << "CREATE SIGNAL WS" << endl;

  MakeSigWS(w, fileBaseName, signalname,signalsample,cat_names, cuts);

  cout << "CREATE BACKGROUND" << endl;
  AddBkgData(w,cat_names,altfunc,cuts);//Error if I don't use 4 Arguments
    
  cout << "FIT BACKGROUND" << endl;
  fitresults = BkgModelFit(altfunc, w, dobands,cat_names, cuts);
  
  cout << "CREATE BACKGROUND WS" << endl;
  MakeBkgWS(altfunc,w, fileBkgName,cat_names);
  
  cout << "CREATE DATACARD" << endl;

  MakeDataCard_1Channel(altfunc, w, fileBaseName, fileBkgName, CHANNEL, signalname, signalsample, cat_names, mass);
  
  cout << "MAKE PLOTS" << endl;
  
  // Make plots for data and fit results
  // MakePlots(w, mass, fitresults, signalname,cat_names);
  // MakePrettyPlots(w, mass, fitresults, signalname,cat_names);
  
  cout << "DONE WITH SIGNAL " << signalname << " FOR MASS POINT " << mass <<" GEV " << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "" << endl;
  cout << "" << endl;

  return;
}



void AddSigData(RooWorkspace* w, Float_t mass, int signalsample, std::vector<string> cat_names, string cut) {
  
  std::cout << "CHANNEL == " << CHANNEL << std::endl;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;

  TString inDir   = directory+"MiniTrees/SignalUHH/";

  Float_t MASS(mass);
  // Float_t sqrts(21);
  Float_t sqrts(21);
  
  cout << " min = "<< ncat_min<<" ncat = "<<ncat << " max = " << ncat_min+ncat<<endl;  

  //****************************//
  // Signal Data Set
  //****************************//

  // Variables
  RooArgSet* ntplVars = defineVariables();
  RooRealVar weightVar("weightVar","",1,0,1000);

  int iMass = abs(mass); 
  TString sigfilename;
  if (signalsample==0) {
    sigfilename = TString(Form("dijetUHH_13TeV_graviton_%s_Interpolated%d_miniTree.root", cut.c_str(), iMass));
  }
  if (signalsample==1) {
    sigfilename = TString(Form("dijetUHH_13TeV_radion_%s_Interpolated%d_miniTree.root", cut.c_str(), iMass));
  }
  if (signalsample==10) {
    sigfilename = TString(Form("dijetUHH_13TeV_%s_miniTree.root", cut.c_str()));
  }
  TFile sigFile1(inDir+sigfilename);  
  sigFile1.Print();
  std::cout<< "Opening signal file: " <<  sigFile1.GetName() << std::endl;
  TTree* sigTree1 = (TTree*) sigFile1.Get("TCVARS");
  std::cout<< "Opening signal tree: " <<  sigTree1->GetName() << std::endl;

  // add weight var into the list of ntuple variables
  std::cout<< "Adding weight variable into the list of ntuple variables " << std::endl;
  weightVar.setVal(1.);
  ntplVars->add(RooArgList(weightVar));

  // common preselection cut
  TString mainCut("1");


  //****************************//
  // Signal  Data Set
  //****************************//
  // Create non scaled signal dataset composed with  different productions 
  // according to their cross sections

  RooDataSet sigScaled("sigScaled","dataset",sigTree1,*ntplVars,mainCut,"evWeight");

  sigScaled.Print("v");

  RooDataSet* sigToFit[21];
  std::cout<< "Looping over N categories: " <<  ncat_min+ncat << std::endl;
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    std::cout<< "Category: " <<  c << std::endl;
    std::cout<< "Name: " <<  TString::Format("Sig_%s",cat_names.at(c).c_str()) << std::endl;
    sigToFit[c] =  (RooDataSet*) sigScaled.reduce(*w->var("mgg13TeV"),mainCut+TString::Format(" && categories==%d",c));
    w->import(*sigToFit[c],Rename(TString::Format("Sig_%s",cat_names.at(c).c_str())));
  }

  std::cout<< "Create full signal data set without categorization: "<< std::endl;
  // Create full signal data set without categorization
  RooDataSet* sigToFitAll  = (RooDataSet*) sigScaled.reduce(*w->var("mgg13TeV"),mainCut);
  w->import(*sigToFitAll,Rename("Sig"));

  // Create weighted signal dataset composed with  different 
  // production processes according to their cross sections
  // no common preselection cut applied yet 

  RooRealVar *weightVar1 = dynamic_cast<RooRealVar*>( &(*ntplVars)["evWeight"] );
  RooRealVar *weightVar2 = dynamic_cast<RooRealVar*>(&(*ntplVars)["normWeight"]);
  RooFormulaVar *weightVar3 = new RooFormulaVar( "weight3", "", "@0*@1", RooArgList(*weightVar1, *weightVar2));

  weightVar.setVal(1.);
  ntplVars->setRealValue("normWeight", 1.);
  RooDataSet sigWeightedTmp1("sigData","dataset",sigTree1,*ntplVars,mainCut,"weightVar");
  RooRealVar *weightX = (RooRealVar*) sigWeightedTmp1.addColumn(*weightVar3) ;
  RooDataSet sigWeighted("sigData","dataset",
  RooArgList((*ntplVars)["mgg13TeV"],
  (*ntplVars)["categories"],*weightX),
  Import(sigWeightedTmp1),WeightVar(*weightX));

  cout << "---- nX:  " << sigWeighted.sumEntries() << endl; 
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    Float_t nExpEvt = sigWeighted.reduce(*w->var("mgg13TeV"),TString::Format("categories==%d",c))->sumEntries();
    cout << TString::Format("nEvt exp.  %s : ",cat_names.at(c).c_str()) << nExpEvt << endl; 
  }

  sigWeighted.Print("v");


  // apply a common preselection cut;
  // split into categories;

  RooDataSet* signal[21];
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    signal[c] =  (RooDataSet*) sigWeighted.reduce(*w->var("mgg13TeV"),mainCut+TString::Format(" && categories==%d",c));
    w->import(*signal[c],Rename(TString::Format("SigWeight_%s",cat_names.at(c).c_str())));
  }
  // Create full weighted signal data set without categorization
  RooDataSet* signalAll  = (RooDataSet*) sigWeighted.reduce(*w->var("mgg13TeV"),mainCut);
  w->import(*signalAll, Rename("SigWeight"));

}


void AddBkgData(RooWorkspace* w, std::vector<string> cat_names, std::string altfunc, string cut) {
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;

  TString inDir   = directory+"/MiniTrees/DataUHH/";

  // common preselection cut
  TString mainCut("1");

  Float_t minMassFit(MMIN),maxMassFit(MMAX); 


  //****************************//
  // CMS Data Set
  //****************************//
  // retrieve the data tree;
  // no common preselection cut applied yet; 

	TString name ("dijetUHH_13TeV_miniTree.root");
	if(cut!=""){
		 name=TString("dijetUHH_13TeV_miniTree_"+cut+".root");
	}
	std::cout << " take file for mini-Trees : "<< name << std::endl;
	TFile dataFile(inDir+name);
  
  
  TTree* dataTree     = (TTree*) dataFile.Get("TCVARS");

  // Variables
  RooArgSet* ntplVars = defineVariables();

  RooDataSet Data("Data","dataset",dataTree,*ntplVars,"","normWeight");

  // apply a common preselection cut;
  // split into NCAT  categories;

  
  RooDataSet* dataToFit[21];
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    // Real data
    dataToFit[c]   = (RooDataSet*) Data.reduce(*w->var("mgg13TeV"),mainCut+TString::Format(" && categories==%d",c));
    std::cout << " make data workspace " << std::endl;
    std::cout << " category : " << c << std::endl;
    std::cout << " nevents  : " << dataToFit[c]->sumEntries() << std::endl; 
    w->import(*dataToFit[c],Rename(TString::Format("Data_%s",cat_names.at(c).c_str())));
  }

  // Create full data set without categorization
  RooDataSet* data    = (RooDataSet*) Data.reduce(*w->var("mgg13TeV"),mainCut);
  w->import(*data, Rename("Data"));
  data->Print("v");

}

void SigModelFit(RooWorkspace* w, Float_t mass, TString signalname, std::vector<string> cat_names) {
  
  // Int_t ncat = NCAT;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;
  
  std::cout << CHANNEL << std::endl;
  Float_t MASS(mass);

  //******************************************
  // Fit signal with model pdfs
  //******************************************
  // retrieve pdfs and datasets from workspace to fit with pdf models



  RooDataSet* sigToFit[21];
  RooAbsPdf* jjSig[21];

  Float_t minMassFit(MMIN),maxMassFit(MMAX); 


  // Fit Signal 

  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    
    sigToFit[c]  = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    std::cout << sigToFit[c] << std::endl;

    RooRealVar* m0         = new RooRealVar( "jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str()), MASS, 0.8*MASS, 1.2*MASS);
    RooRealVar* gm0        = new RooRealVar( "jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str()), MASS, 0.8*MASS, 1.2*MASS);
    RooRealVar* sigma      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str()), MASS*0.05 ,20., 700.);
    RooRealVar* scalesigma = new RooRealVar( "jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str()), 2., 1.2, 10.);
    RooRealVar* alpha      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str()), 1.85288, 0.0, 20);
    RooRealVar* sig_n      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str()), 129.697, 0., 300);
    RooRealVar* frac       = new RooRealVar( "jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str()), 0.0, 0.0, 0.35);
    
    RooFormulaVar* gsigma  = new RooFormulaVar( "jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str()),"jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str()),"@0*@1", RooArgList( *sigma, *scalesigma ));
    
    RooGaussian* gaus   = new RooGaussian( "jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str()), "jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str()), *w->var("mgg13TeV") ,*m0,*gsigma);
    RooCBShape* cb      = new RooCBShape ( "jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str()), "jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str()), *w->var("mgg13TeV") ,*m0 , *sigma, *alpha, *sig_n);
    RooAddPdf* sigmodel = new RooAddPdf  ( signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , RooArgList( *gaus, *cb ), RooArgList(*frac),1);
    
    jjSig[c] = (RooAbsPdf*)  sigmodel;
    jjSig[c] -> fitTo(*sigToFit[c],Range(mass*0.8,mass*1.2),SumW2Error(kTRUE),PrintEvalErrors(-1),Save(kTRUE));
      
    cout<<"FIT PASSED! Start importing and fixing parameters" <<endl;
    w->import(*sigmodel  );
    w->import(*gaus      );
    w->import(*cb        );
    w->import(*m0        );
    w->import(*gm0       );
    w->import(*sigma     );
    w->import(*scalesigma);
    w->import(*gsigma    );
    w->import(*alpha     );
    w->import(*sig_n     );
    w->import(*frac      );
      
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str())))->setConstant(true);
    // ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str())))->setConstant(true);
    
  }
}

void SigModelFitaQGC(RooWorkspace* w, Float_t mass, TString signalname, std::vector<string> cat_names) {
  
  // Int_t ncat = NCAT;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;
  
  std::cout << CHANNEL << std::endl;
  Float_t MASS(mass);

  //******************************************//
  // Fit signal with model pdfs
  //******************************************//
  // retrieve pdfs and datasets from workspace to fit with pdf models

  RooDataSet* sigToFit[21];
  RooAbsPdf* jjSig[21];

  Float_t minMassFit(MMIN),maxMassFit(MMAX); 

  RooRealVar* mgg = w->var("mgg13TeV");  
  mgg->setUnit("GeV");

  // Fit Signal -> EFT par4 / SM -> par3
  if(TString(signalname).Contains("_mjj")){

  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    
    sigToFit[c]  = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    std::cout << sigToFit[c] << std::endl;

    RooRealVar* m0         = new RooRealVar( "jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str()), MASS, 0.8*MASS, 1.2*MASS);
    RooRealVar* gm0        = new RooRealVar( "jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str()), MASS, 0.8*MASS, 1.2*MASS);
    RooRealVar* sigma      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str()), MASS*0.05 ,20., 700.);
    RooRealVar* scalesigma = new RooRealVar( "jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str()), 2., 1.2, 10.);
    RooRealVar* alpha      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str()), 1.85288, 0.0, 20);
    RooRealVar* sig_n      = new RooRealVar( "jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str()), 129.697, 0., 300);
    RooRealVar* frac       = new RooRealVar( "jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str()), "jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str()), 0.0, 0.0, 0.35);
    
    RooFormulaVar* gsigma  = new RooFormulaVar( "jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str()),"jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str()),"@0*@1", RooArgList( *sigma, *scalesigma ));
    
    RooGaussian* gaus   = new RooGaussian( "jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str()), "jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str()), *w->var("mgg13TeV") ,*m0,*gsigma);
    RooCBShape* cb      = new RooCBShape ( "jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str()), "jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str()), *w->var("mgg13TeV") ,*m0 , *sigma, *alpha, *sig_n);
    RooAddPdf* sigmodel = new RooAddPdf  ( signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , RooArgList( *gaus, *cb ), RooArgList(*frac),1);
    
    jjSig[c] = (RooAbsPdf*)  sigmodel;
    jjSig[c] -> fitTo(*sigToFit[c],Range(mass*0.8,mass*1.2),SumW2Error(kTRUE),PrintEvalErrors(-1),Save(kTRUE));
      
    cout<<"FIT PASSED! Start importing and fixing parameters" <<endl;
    w->import(*sigmodel  );
    w->import(*gaus      );
    w->import(*cb        );
    w->import(*m0        );
    w->import(*gm0       );
    w->import(*sigma     );
    w->import(*scalesigma);
    w->import(*gsigma    );
    w->import(*alpha     );
    w->import(*sig_n     );
    w->import(*frac      );
      
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_m0_%s"    ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_gm0_%s"   ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_sigma_%s" ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_scalesigma_%s",cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_alpha_%s" ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_n_%s"     ,cat_names.at(c).c_str())))->setConstant(true);
    // ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_gsigma_%s",cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var("jj_"+signalname+TString::Format("_sig_frac_%s"  ,cat_names.at(c).c_str())))->setConstant(true);
    
  }
}


    //old bzw my version
    /*
    for(int c = ncat_min; c < ncat_min+ncat; ++c){
    
    sigToFit[c] = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    std::cout << sigToFit[c] <<std::endl;

    w->factory(TString::Format("sig_fit_slope1_%s[5.,-100.,100.]",cat_names.at(c).c_str()));
    w->factory(TString::Format("sig_fit_slope2_%s[5.,-100.,100.]",cat_names.at(c).c_str()));
    w->factory(TString::Format("sig_fit_slope3_%s[5.,0.,50.]",cat_names.at(c).c_str()));
    w->factory(TString::Format("sig_fit_slope4_%s[100.,-1000.,1400.]",cat_names.at(c).c_str()));

    RooFormulaVar *p1mod = new RooFormulaVar(TString::Format("sig_p1mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope1_%s",cat_names.at(c).c_str())));
    RooFormulaVar *p2mod = new RooFormulaVar(TString::Format("sig_p2mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope2_%s",cat_names.at(c).c_str())));
    RooFormulaVar *p3mod = new RooFormulaVar(TString::Format("sig_p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope3_%s",cat_names.at(c).c_str())));
    RooFormulaVar *p4mod = new RooFormulaVar(TString::Format("sig_p4mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope4_%s",cat_names.at(c).c_str())));
    
    //RooFormulaVar *sqrtS = new RooFormulaVar(TString::Format("sqrtS_%s",cat_names.at(c).c_str()),"","@0",*w->var("sqrtS"));
    //RooFormulaVar *x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));

    RooAbsPdf* sigmodel;
    
    //x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));
    //maybe not necessary
    p1mod = new RooFormulaVar(TString::Format("sig_p1mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope1_%s",cat_names.at(c).c_str())));
    p2mod = new RooFormulaVar(TString::Format("sig_p2mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope2_%s",cat_names.at(c).c_str())));
    p3mod = new RooFormulaVar(TString::Format("sig_p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope3_%s",cat_names.at(c).c_str())));
   // p4mod = new RooFormulaVar(TString::Format("sig_p4mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope4_%s",cat_names.at(c).c_str())));
    //fourpar='[0]*TMath::Power((1-(x/13000)),[1])/TMath::Power(x/13000,[2]+[3]*TMath::Log10(x/13000))' it was : ( @1*pow(1-@0 + @4*pow(@0,2),@2) ) / ( pow(@0/13000.,@3) )   BG: "1./pow((@0/@1), @2)",                                                 
    sigmodel = new RooGenericPdf( signalname+"_jj"+TString::Format("_sig_%s",cat_names.at(c).c_str()), signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str()), "( pow((1-@0/13000), @1) / pow(@0/13000, @2 +@3*log10(@0/13000)) )", RooArgList(*mgg, *p1mod, *p2mod, *p3mod));

    jjSig[c] = (RooAbsPdf*)  sigmodel;
    jjSig[c] -> fitTo(*sigToFit[c],Range(MMIN,MMAX),SumW2Error(kTRUE),PrintEvalErrors(-1),Save(kTRUE));

    cout<<"FIT PASSED! Start importing and fixing parameters" <<endl; 
    
    w->import(*sigmodel);
    w->import(*p1mod  );
    w->import(*p2mod  );
    w->import(*p3mod  );
    //w->import(*p4mod  );
    
    ((RooRealVar*) w->var(TString::Format("sig_fit_slope1_%s"    ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var(TString::Format("sig_fit_slope2_%s"   ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var(TString::Format("sig_fit_slope3_%s" ,cat_names.at(c).c_str())))->setConstant(true);
   // ((RooRealVar*) w->var(TString::Format("sig_fit_slope4_%s" ,cat_names.at(c).c_str())))->setConstant(true)  
    }
    */
  }else if(TString(signalname).Contains("_pT")){
    for(int c = ncat_min; c < ncat_min+ncat; ++c){
    
    sigToFit[c] = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    std::cout << sigToFit[c] <<std::endl;

    w->factory(TString::Format("sig_fit_slope1_%s[5.,-100.,100.]",cat_names.at(c).c_str()));
    w->factory(TString::Format("sig_fit_slope2_%s[5.,-100.,100.]",cat_names.at(c).c_str()));
    //w->factory(TString::Format("sig_fit_slope3_%s[5.,0.,50.]",cat_names.at(c).c_str()));

    RooFormulaVar *p1mod = new RooFormulaVar(TString::Format("sig_p1mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope1_%s",cat_names.at(c).c_str())));
    RooFormulaVar *p2mod = new RooFormulaVar(TString::Format("sig_p2mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope2_%s",cat_names.at(c).c_str())));
    //RooFormulaVar *p3mod = new RooFormulaVar(TString::Format("sig_p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope3_%s",cat_names.at(c).c_str())));
    //RooFormulaVar *sqrtS = new RooFormulaVar(TString::Format("sqrtS_%s",cat_names.at(c).c_str()),"","@0",*w->var("sqrtS"));
    //RooFormulaVar *x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));
    RooAbsPdf* sigmodel;
    //x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));
    //maybe not necessary
    p1mod = new RooFormulaVar(TString::Format("sig_p1mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope1_%s",cat_names.at(c).c_str())));
    p2mod = new RooFormulaVar(TString::Format("sig_p2mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope2_%s",cat_names.at(c).c_str())));
    //p3mod = new RooFormulaVar(TString::Format("sig_p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("sig_fit_slope3_%s",cat_names.at(c).c_str())));
    //Fitfunction from python                                                                                                                              three par = '[0]*TMath::Power((1-(x/13000)),[1])/TMath::Power(x/13000, [2])'
    sigmodel = new RooGenericPdf( signalname+"_jj"+TString::Format("_sig_%s",cat_names.at(c).c_str()), signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str()), "pow((1-@0/13000), @1) / pow(@0/13000, @2)", RooArgList(*mgg, *p1mod, *p2mod));

    jjSig[c] = (RooAbsPdf*)  sigmodel;
    jjSig[c] -> fitTo(*sigToFit[c],Range(MMIN,MMAX),SumW2Error(kTRUE),PrintEvalErrors(-1),Save(kTRUE));

    cout<<"FIT PASSED! Start importing and fixing parameters" <<endl; 
    
    w->import(*sigmodel);
    w->import(*p1mod  );
    w->import(*p2mod  );
    //w->import(*p3mod  );
    
    ((RooRealVar*) w->var(TString::Format("sig_fit_slope1_%s"    ,cat_names.at(c).c_str())))->setConstant(true);
    ((RooRealVar*) w->var(TString::Format("sig_fit_slope2_%s"   ,cat_names.at(c).c_str())))->setConstant(true);
    //((RooRealVar*) w->var(TString::Format("sig_fit_slope3_%s" ,cat_names.at(c).c_str())))->setConstant(true);
  
   }
  }
}
vector<RooFitResult*> BkgModelFit(std::string altfunc,RooWorkspace* w, Bool_t dobands, std::vector<string> cat_names, TString cuts) {
  
  // Int_t ncat = NCAT;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;

  //******************************************//
  // Fit background with model pdfs
  //******************************************//

  // retrieve pdfs and datasets from workspace to fit with pdf models


  RooDataSet* data[21];
  vector<RooFitResult*> fitresult;
  for( int i=0;i<21;i++){
   fitresult.push_back(0);   
  }
  RooPlot* plotbkg_fit[21];

  // dobands and dosignal
  RooDataSet* signal[21];
  RooAbsPdf*  jjSig[21];


  Float_t minMassFit(MMIN),maxMassFit(MMAX); 

  // Fit data with background pdf for data limit || for pT: '[0]/TMath::Power(1-(x/13000),[1])/TMath::Power(x/13000,[2])'

  RooRealVar* mgg     = w->var("mgg13TeV");  
  mgg->setUnit("GeV");
  
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    int mainCategory=c;
    if (correlateBackgroundParametersBetweenCategories) mainCategory=0;
    
    data[c]   = (RooDataSet*) w->data(TString::Format("Data_%s",cat_names.at(c).c_str()));

    w->factory(TString::Format("bkg_fit_slope1_%s[5.,-100.,100.]",cat_names.at(mainCategory).c_str()));
    w->factory(TString::Format("bkg_fit_slope2_%s[5.,-100.,100.]",cat_names.at(mainCategory).c_str()));
    w->factory(TString::Format("bkg_fit_slope3_%s[5.,0.,50.]",cat_names.at(mainCategory).c_str()));
    w->factory(TString::Format("bkg_fit_slope4_%s[100.,-1000.,1000.]",cat_names.at(mainCategory).c_str()));
 
    RooFormulaVar *p1mod = new RooFormulaVar(TString::Format("p1mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())));
    RooFormulaVar *p2mod = new RooFormulaVar(TString::Format("p2mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())));
    RooFormulaVar *p3mod = new RooFormulaVar(TString::Format("p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())));
    RooFormulaVar *p4mod = new RooFormulaVar(TString::Format("p4mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())));
     
    RooFormulaVar *sqrtS = new RooFormulaVar(TString::Format("sqrtS_%s",cat_names.at(c).c_str()),"","@0",*w->var("sqrtS"));
    RooFormulaVar *x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));

    // 2 parameter fit
    RooAbsPdf* bkg_fitTmp = bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "1./pow((@0/13000), @1)", RooArgList(*mgg, *p1mod)); 
    //RooAbsPdf* bkg_fitTmp = bkg_fitTmp;
    /*
    //================== make fit with alternate function =====================================
    if (altfunc.find("3par")!=std::string::npos)
    {
    //============== use 3 parameter funtion as alternative function ====================================
    bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "pow(1-@0, @1)/pow(@0, @2)", RooArgList(*x, *p1mod, *p2mod)); // 3 parameter fitnew 
    }
    if (altfunc.find("Exp")!=std::string::npos)
    {
      //============== use levelled exponential as alternative function ========================================
       RooFormulaVar *x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0",RooArgList(*mgg));
       p3mod = new RooFormulaVar(TString::Format("p3mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())));
       bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "exp(-(@0-@1)/(@2+@3*(@0-@1)))", RooArgList(*x, *p1mod, *p2mod, *p3mod));
    }
    if (altfunc.find("alt")!=std::string::npos)
    {
    //============== use alt. four parameter funtion as alternative function ====================================
    x = new RooFormulaVar(TString::Format("x_%s",cat_names.at(c).c_str()),"","@0/@1",RooArgList(*mgg, *sqrtS));
    p4mod = new RooFormulaVar(TString::Format("p4mod_%s",cat_names.at(c).c_str()),"","@0",*w->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())));
    bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "( @1*pow(1-@0 + @4*pow(@0,2),@2) ) / ( pow(@0/13000.,@3) )", RooArgList(*x, *p1mod, *p2mod, *p3mod,*p4mod));
    }
    */
    if(TString(cuts).Contains("_mjj")){
    RooAbsReal* bkg_fitTmp2  = new RooRealVar(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str()),"",data[c]->sumEntries(),1.0,1000000000);
    RooAbsPdf*  bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "1./pow((@0/13000), @1)", RooArgList(*mgg, *p1mod));
    w->import(*bkg_fitTmp);
    w->import(*bkg_fitTmp2);
 
    if (c==mainCategory)
      fitresult[c] = bkg_fitTmp->fitTo(*data[c], Strategy(1),Minos(kFALSE), Range(minMassFit,maxMassFit),SumW2Error(kTRUE), Save(kTRUE),PrintEvalErrors(-1));
    if(constrainBackgroundParametersToSimulation) {
    ((RooRealVar*) w->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
    ((RooRealVar*) w->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
    ((RooRealVar*) w->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
    ((RooRealVar*) w->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
    }
    }else if(TString(cuts).Contains("_pT")){
      RooAbsReal* bkg_fitTmp2  = new RooRealVar(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str()),"",data[c]->sumEntries(),1.0,1000000000);
      RooAbsPdf* bkg_fitTmp = new RooGenericPdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()), "pow(1-@0/13000, @1)/pow(@0/13000, @2)", RooArgList(*mgg, *p1mod, *p2mod)); // 3 parameter fitnew 
          
      w->import(*bkg_fitTmp);
      w->import(*bkg_fitTmp2);

      if (c==mainCategory)
      fitresult[c] = bkg_fitTmp->fitTo(*data[c], Strategy(1),Minos(kFALSE), Range(minMassFit,maxMassFit),SumW2Error(kTRUE), Save(kTRUE),PrintEvalErrors(-1));
      if(constrainBackgroundParametersToSimulation) {
      ((RooRealVar*) w->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
      ((RooRealVar*) w->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->setConstant(true); //newStrategy
      }
    }
    
    //************************************************//
    // Plot jj background fit results per categories 
    //************************************************//
    // Plot Background Categories 
    //****************************//

    TCanvas* ctmp = new TCanvas("ctmp","jj Background Categories",0,0,500,500);
    Int_t nBinsMass(80);
    plotbkg_fit[c] = mgg->frame(nBinsMass);
    data[c]->plotOn(plotbkg_fit[c],LineColor(kWhite),MarkerColor(kWhite));    

    bkg_fitTmp->plotOn(plotbkg_fit[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1)); 
    data[c]->plotOn(plotbkg_fit[c]);    

    plotbkg_fit[c]->Draw();  

    //********************************************************************************//

    if (dobands) {

      RooAbsPdf *cpdf; cpdf = bkg_fitTmp;
      TGraphAsymmErrors *onesigma = new TGraphAsymmErrors();
      TGraphAsymmErrors *twosigma = new TGraphAsymmErrors();
      
      RooRealVar *nlim = new RooRealVar(TString::Format("nlim%d",c),"",0.0,0.0,10.0);
      nlim->removeRange();
      
      RooCurve *nomcurve = dynamic_cast<RooCurve*>(plotbkg_fit[c]->getObject(1));
      
      for (int i=1; i<(plotbkg_fit[c]->GetXaxis()->GetNbins()+1); ++i) {
        double lowedge = plotbkg_fit[c]->GetXaxis()->GetBinLowEdge(i);
        double upedge  = plotbkg_fit[c]->GetXaxis()->GetBinUpEdge(i);
        double center  = plotbkg_fit[c]->GetXaxis()->GetBinCenter(i);
	
        double nombkg = nomcurve->interpolate(center);
        nlim->setVal(nombkg);
        mgg->setRange("errRange",lowedge,upedge);
        RooAbsPdf *epdf = 0;
        epdf = new RooExtendPdf("epdf","",*cpdf,*nlim,"errRange");
	
        RooAbsReal *nll = epdf->createNLL(*(data[c]),Extended());
        RooMinimizer minim(*nll);
        minim.setStrategy(0);
        double clone = 1.0 - 2.0*RooStats::SignificanceToPValue(1.0);
        double cltwo = 1.0 - 2.0*RooStats::SignificanceToPValue(2.0);
	
        minim.migrad();
        minim.minos(*nlim);
        // printf("errlo = %5f, errhi = %5f\n",nlim->getErrorLo(),nlim->getErrorHi());
	
        onesigma->SetPoint(i-1,center,nombkg);
        onesigma->SetPointError(i-1,0.,0.,-nlim->getErrorLo(),nlim->getErrorHi());
	
        minim.setErrorLevel(0.5*pow(ROOT::Math::normal_quantile(1-0.5*(1-cltwo),1.0), 2)); // the 0.5 is because qmu is -2*NLL
        // eventually if cl = 0.95 this is the usual 1.92!      
	
	
        minim.migrad();
        minim.minos(*nlim);
	
        twosigma->SetPoint(i-1,center,nombkg);
        twosigma->SetPointError(i-1,0.,0.,-nlim->getErrorLo(),nlim->getErrorHi());
	
	
        delete nll;
        delete epdf;
	
      }
      mgg->setRange("errRange",minMassFit,maxMassFit);
      
      twosigma->SetLineColor(kGreen);
      twosigma->SetFillColor(kGreen);
      twosigma->SetMarkerColor(kGreen);
      twosigma->Draw("L3 SAME");
      
      onesigma->SetLineColor(kYellow);
      onesigma->SetFillColor(kYellow);
      onesigma->SetMarkerColor(kYellow);
      onesigma->Draw("L3 SAME");
      
      plotbkg_fit[c]->Draw("SAME"); 
     
    }

  }
  return fitresult;


}


void SetConstantParams(const RooArgSet* params) {

  TIterator* iter(params->createIterator());
  for (TObject *a = iter->Next(); a != 0; a = iter->Next()) {
    RooRealVar *rrv = dynamic_cast<RooRealVar *>(a);
    if (rrv) { rrv->setConstant(true); std::cout << " " << rrv->GetName(); }
  }  

}

void MakePlots(RooWorkspace* w, Float_t mass, vector<RooFitResult*> fitresults, TString signalname, std::vector<string> cat_names) {
  
  cout << "Start plotting" << endl;
  
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;

  // retrieve data sets from the workspace
  RooDataSet* dataAll         = (RooDataSet*) w->data("Data");
  RooDataSet* signalAll       = (RooDataSet*) w->data("Sig");

  RooDataSet* data[21];  
  RooDataSet* signal[21];
  RooAbsPdf*  jjGaussSig[21];
  RooAbsPdf*  jjCBSig[21];
  RooAbsPdf*  jjSig[21];
  RooAbsPdf*  bkg_fit[21];  

  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    data[c]         = (RooDataSet*) w->data(TString::Format("Data_%s",cat_names.at(c).c_str()));
    signal[c]       = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    jjGaussSig[c]  = (RooAbsPdf*)  w->pdf(TString::Format("jj_GaussSig_%s",cat_names.at(c).c_str()));
    jjCBSig[c]     = (RooAbsPdf*)  w->pdf(TString::Format("jj_CBSig_%s",cat_names.at(c).c_str()));
    jjSig[c]       = (RooAbsPdf*)  w->pdf(signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str()));
    bkg_fit[c]       = (RooAbsPdf*)  w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()));
  }

  // retrieve mass observable from the workspace
  RooRealVar* mgg     = w->var("mgg13TeV");  
  mgg->setUnit("GeV");

  //********************************************//
  // Plot jj signal fit results per categories 
  //********************************************//
  // Plot Signal Categories 
  //****************************//

  Float_t minMassFit(MMIN),maxMassFit(MMAX);
  Float_t MASS(mass);
  Int_t nBinsMass(100);
  gStyle->SetOptTitle(0);
  
  RooPlot* plotjj[21];
  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    plotjj[c] = mgg->frame(Range(minMassFit,maxMassFit),Bins(nBinsMass));
    signal[c] ->plotOn(plotjj[c],LineColor(kWhite),MarkerColor(kWhite),PrintEvalErrors(-1));    

    jjSig[c]  ->plotOn(plotjj[c],LineColor(col->GetColor("#636363")),PrintEvalErrors(-1));
    jjSig[c]  ->plotOn(plotjj[c],Components("jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str())),LineStyle(kDashed),LineColor(col->GetColor("#99d8c9")),PrintEvalErrors(-1));
    jjSig[c]  ->plotOn(plotjj[c],Components("jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str())),LineStyle(kDashed),LineColor(col->GetColor("#fdbb84")),PrintEvalErrors(-1));
    

    jjSig[c]   ->paramOn(plotjj[c]);
    signal[c]  ->plotOn(plotjj[c],PrintEvalErrors(-1));

    Double_t chi2 = plotjj[c]->chiSquare();

  
    int W = 800;
    int H = 600;
    int H_ref = 600; 
    int W_ref = 800; 
    float T = 0.08*H_ref;
    float B = 0.12*H_ref; 
    float L = 0.12*W_ref;
    float R = 0.04*W_ref;
    
    TCanvas* dummy = new TCanvas("dummy", "dummy",50,50,W,H);
    dummy->SetFillColor(0);
    dummy->SetBorderMode(0);
    dummy->SetFrameFillStyle(0);
    dummy->SetFrameBorderMode(0);
    dummy->SetLeftMargin( L/W );
    dummy->SetRightMargin( R/W );
    dummy->SetTopMargin( T/H );
    dummy->SetBottomMargin( B/H );
    dummy->SetTickx(0);
    dummy->SetTicky(0);
    
    TH1F *hist = new TH1F("hist", "hist", 400, minMassFit, maxMassFit);
 
    plotjj[c]->SetTitle("");      
    plotjj[c]->SetMinimum(0.0);
    plotjj[c]->SetMaximum(1.40*plotjj[c]->GetMaximum());
    plotjj[c]->GetXaxis()->SetTitle("Dijet invariant mass (GeV)");
    plotjj[c]->GetYaxis()->SetTitleOffset(1.1);

    TCanvas* ctmp = new TCanvas("ctmp","jj Background Categories",0,0,500,500);
    plotjj[c]->Draw();  
    //    hist->Draw("same");
    
    plotjj[c]->Draw("SAME");
    TLegend *legmc = new TLegend(0.570,0.72,0.85,0.87);
    legmc->AddEntry(plotjj[c]->getObject(5),"CMS data","LPE");
    legmc->AddEntry(plotjj[c]->getObject(1),"Total PDF","L");
    legmc->AddEntry(plotjj[c]->getObject(3),"Crystal Ball comp.","L");
    legmc->AddEntry(plotjj[c]->getObject(2),"Gaussian comp.","L");
    
    legmc->SetBorderSize(0);
    legmc->SetFillStyle(0);
    legmc->Draw();
    
    TLatex *lat2 = new TLatex(minMassFit+1.5,0.75*plotjj[c]->GetMaximum(),cat_names.at(c).c_str());
    lat2->Draw();

    int iMass = abs(mass);

    ctmp->SaveAs(directory+"/plots/sigmodel_"+signalname+TString::Format("%d_%s.pdf", iMass, cat_names.at(c).c_str()));
    ctmp->SaveAs(directory+"/plots/sigmodel_"+signalname+TString::Format("%d_%s.root", iMass, cat_names.at(c).c_str()));

    
  }


  //************************************************//
  // Plot jj background fit results per categories 
  //************************************************//
  // Plot Background Categories 
  //****************************//

  TCanvas* c4 = new TCanvas("c4","jj Background Categories",0,0,2000,2000);
  c4->Divide(3,7);

  RooPlot* plotbkg_fit[21];
  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    plotbkg_fit[c] = mgg->frame(Range(minMassFit,maxMassFit),Bins(nBinsMass));
    data[c]->plotOn(plotbkg_fit[c],LineColor(kWhite),MarkerColor(kWhite));    
    bkg_fit[c]->plotOn(plotbkg_fit[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1)); 
    data[c]->plotOn(plotbkg_fit[c]);    
    bkg_fit[c]->paramOn(plotbkg_fit[c],Layout(0.4,0.9,0.9), Format("NEU",AutoPrecision(4)));
    plotbkg_fit[c]->getAttText()->SetTextSize(0.03);
    c4->cd(c+1);
    plotbkg_fit[c]->Draw();  
    gPad->SetLogy(1);
    plotbkg_fit[c]->SetAxisRange(0.1,plotbkg_fit[c]->GetMaximum()*1.5,"Y");
  }


  // c4->SaveAs((directory+"/plots/backgrounds_log.pdf").c_str());
	c4->SaveAs(directory+"/plots/backgrounds_log_"+signalname+".pdf");

  
  TCanvas* c5 = new TCanvas("c5","jj Background Categories",0,0,2000,2000);
  c5->Divide(3,7);
  
  RooPlot* plotbkg_fit2[21];
  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    plotbkg_fit2[c] = mgg->frame(nBinsMass);
    data[c]->plotOn(plotbkg_fit[c],LineColor(kWhite),MarkerColor(kWhite));    
    bkg_fit[c]->plotOn(plotbkg_fit[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1)); 
    data[c]->plotOn(plotbkg_fit[c]);    
    bkg_fit[c]->paramOn(plotbkg_fit[c], ShowConstants(true), Layout(0.4,0.9,0.9), Format("NEU",AutoPrecision(4)));
    plotbkg_fit[c]->getAttText()->SetTextSize(0.03);
    c5->cd(c+1);
    plotbkg_fit[c]->Draw();  
  }

  // c5->SaveAs((directory+"/plots/backgrounds.pdf").c_str());
  c5->SaveAs(directory+"/plots/backgrounds_"+signalname+".pdf");

}

void MakePrettyPlots(RooWorkspace* w, Float_t mass, vector<RooFitResult*> fitresults, TString signalname, std::vector<string> cat_names) {
  
  cout << "Start plotting" << endl;
  
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;

  // retrieve data sets from the workspace
  RooDataSet* dataAll         = (RooDataSet*) w->data("Data");
  RooDataSet* signalAll       = (RooDataSet*) w->data("Sig");

  RooDataSet* data[21];  
  RooDataSet* signal[21];
  RooAbsPdf*  jjGaussSig[21];
  RooAbsPdf*  jjCBSig[21];
  RooAbsPdf*  jjSig[21];
  RooAbsPdf*  bkg_fit[21];  

  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    data[c]         = (RooDataSet*) w->data(TString::Format("Data_%s",cat_names.at(c).c_str()));
    signal[c]       = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
    jjGaussSig[c]  = (RooAbsPdf*)  w->pdf(TString::Format("jj_GaussSig_%s",cat_names.at(c).c_str()));
    jjCBSig[c]     = (RooAbsPdf*)  w->pdf(TString::Format("jj_CBSig_%s",cat_names.at(c).c_str()));
    jjSig[c]       = (RooAbsPdf*)  w->pdf(signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str()));
    bkg_fit[c]       = (RooAbsPdf*)  w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()));
  }

  // retrieve mass observable from the workspace
  RooRealVar* mgg     = w->var("mgg13TeV");  
  mgg->setUnit("GeV");

  //********************************************//
  // Plot jj signal fit results per categories 
  //********************************************//
  // Plot Signal Categories 
  //****************************//

  Float_t minMassFit(MMIN),maxMassFit(MMAX);
  Float_t MASS(mass);
  Int_t nBinsMass(100);
  gStyle->SetOptTitle(0);
  
  RooPlot* plotjj[21];
  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    // plotjj[c] = mgg->frame(Range(minMassFit,maxMassFit),Bins(nBinsMass));
    
    // signal[c] ->plotOn(plotjj[c],LineColor(kWhite),MarkerColor(kWhite),MarkerSize(9),PrintEvalErrors(-1));    
    jjSig[c]  ->plotOn(plotjj[c],LineColor(col->GetColor("#636363")),DrawOption("L"),PrintEvalErrors(-1),Name("signal"),Invisible());

    bkg_fit[c]->plotOn(plotjj[c],LineColor(col->GetColor("#32492b")),PrintEvalErrors(-1),AddTo("signal"));
    // data[c] ->plotOn(plotjj[c],LineColor(kWhite),MarkerColor(kWhite),PrintEvalErrors(-1),AddTo("signal"));    
    data[c] ->plotOn(plotjj[c],PrintEvalErrors(-1),AddTo("signal"));    

    // gx.plotOn(frame1,Normalization( n_phys_sig.getVal() , RooAbsReal::NumEvent ) ,
    // 	      RooFit::DrawOption("F") , RooFit::FillColor(46) , Name("gx_phys") , Invisible() ) ;
    // px.plotOn(frame1,Normalization( n_phys_bg.getVal() , RooAbsReal::NumEvent ) ,
    // 	      RooFit::DrawOption("F") , RooFit::FillColor(46) , AddTo("gx_phys")) ;
    // data->plotOn(frame1) ;
    // px.plotOn(frame1,Normalization( n_phys_bg.getVal() , RooAbsReal::NumEvent ) ,
    // 	      RooFit::DrawOption("F") , RooFit::FillColor(46) , Name("px_phys")) ;

    // jjSig[c]  ->plotOn(plotjj[c],Components("jj_GaussSig"+signalname+TString::Format("_%s",cat_names.at(c).c_str())),LineStyle(kDashed),LineColor(col->GetColor("#99d8c9")),PrintEvalErrors(-1));
    // jjSig[c]  ->plotOn(plotjj[c],Components("jj_CBSig"   +signalname+TString::Format("_%s",cat_names.at(c).c_str())),LineStyle(kDashed),LineColor(col->GetColor("#fdbb84")),PrintEvalErrors(-1));
    
    // RooAddPdf sum("sum","g+a",RooArgList(*jjSig[c],*bkg_fit[c]),RooArgList(*mgg,*mgg)) ;
    // RooAddPdf sum("sum","g+a",RooArgList(*jjSig[c],*bkg_fit[c])) ;
    // sum.plotOn(plotjj[c],LineColor(kRed),LineStyle(kDashed),Range("fitrange"),NormRange("fitrange"));

    // RooAddPdf sigdata("sigdata","sig+data",RooArgList(*signal[0],*data[0])) ;
    // RooAddPdf* model = new RooAddPdf("model","g+a",*jjSig[c],*bkg_fit[c]) ;
    // RooAddPdf* sigdata = new RooAddPdf("sigdata","sig+data",RooArgList(*signal[0],*data[0],"test")) ;
    // RooAddPdf* sigmodel = new RooAddPdf  ( signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())        , RooArgList( *gaus, *cb ), RooArgList(*frac),1);
    // plotboth[c]=mgg->frame(nBinsMass);
    // sigdata->plotOn(plotboth,LineColor(kWhite),MarkerColor(kWhite));
    // model.plotOn(plotjj[c],Components(RooArgSet(*jjSig[c],*bkg_fit[c])),LineColor(kBlue),LineStyle(kDashed),Range("fitrange"),NormRange("fitrange"));
    // model.plotOn(plotboth[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1));
    // sigdata->plotOn(plotboth);
    // plotboth[c]->Draw();

    // c6->SaveAs((directory+"/plots/both_pretty.pdf").c_str());
    // system(("cp "+directory+"/plots/both_pretty.pdf ~/aQGCVVjj/").c_str());


    // jjSig[c]   ->paramOn(plotjj[c]);
    // signal[c]  ->plotOn(plotjj[c],PrintEvalErrors(-1));
  
    int W = 800;
    int H = 600;
    int H_ref = 600; 
    int W_ref = 800; 
    float T = 0.08*H_ref;
    float B = 0.12*H_ref; 
    float L = 0.12*W_ref;
    float R = 0.04*W_ref;
    
    // TCanvas* dummy = new TCanvas("dummy", "dummy",50,50,W,H);
    // dummy->SetFillColor(0);
    // dummy->SetBorderMode(0);
    // dummy->SetFrameFillStyle(0);
    // dummy->SetFrameBorderMode(0);
    // dummy->SetLeftMargin( L/W );
    // dummy->SetRightMargin( R/W );
    // dummy->SetTopMargin( T/H );
    // dummy->SetBottomMargin( B/H );
    // dummy->SetTickx(0);
    // dummy->SetTicky(0);
    
    // TH1F *hist = new TH1F("hist", "hist", 400, minMassFit, maxMassFit);
 
    plotjj[c]->SetTitle("");      
    plotjj[c]->SetMinimum(0.0);
    plotjj[c]->SetMaximum(1.40*plotjj[c]->GetMaximum());
    plotjj[c]->GetXaxis()->SetTitle("Dijet invariant mass (GeV)");
    plotjj[c]->GetYaxis()->SetTitleOffset(1.1);

    TCanvas* ctmp = new TCanvas("ctmp","jj Background Categories",0,0,500,500);
    ctmp->SetFillColor(0);
    // ctmp->SetBorderMode(0);
    // ctmp->SetFrameFillStyle(0);
    // ctmp->SetFrameBorderMode(0);
    ctmp->SetLeftMargin( L/W );
    ctmp->SetRightMargin( R/W );
    ctmp->SetTopMargin( T/H );
    ctmp->SetBottomMargin( B/H );
    ctmp->SetTickx(0);
    ctmp->SetTicky(0);

    plotjj[c]->Draw();  
    //    hist->Draw("same");
    
    // plotjj[c]->Draw("SAME");
  //   TLegend *legmc = new TLegend(0.570,0.72,0.85,0.87);
  //   legmc->AddEntry(plotjj[c]->getObject(5),"CMS data","LPE");
  //   legmc->AddEntry(plotjj[c]->getObject(1),"Total PDF","L");
  //   legmc->AddEntry(plotjj[c]->getObject(3),"Crystal Ball comp.","L");
  //   legmc->AddEntry(plotjj[c]->getObject(2),"Gaussian comp.","L");
    
  //   legmc->SetBorderSize(0);
  //   legmc->SetFillStyle(0);
  //   legmc->Draw();
    
  //   TLatex *lat2 = new TLatex(minMassFit+1.5,0.75*plotjj[c]->GetMaximum(),cat_names.at(c).c_str());
  //   lat2->Draw();

  //   int iMass = abs(mass);

    // ctmp->SaveAs(directory+"/plots/sigmodel_"+signalname+TString::Format("%d_%s_pretty.pdf", iMass, cat_names.at(c).c_str()));
    // system("cp "+directory+"/plots/sigmodel_"+signalname+TString::Format("%d_%s_pretty.pdf", iMass, cat_names.at(c).c_str())+" ~/aQGCVVjj/");
    // ctmp->SaveAs((directory+"/plots/both_pretty.pdf").c_str());
    // system(("cp "+directory+"/plots/both_pretty.pdf ~/aQGCVVjj/").c_str());
  }

  // TCanvas* c6 = new TCanvas("c6","Simultaneous Fit",0,0,2000,2000);
  // RooPlot* plotboth[2];
 
  // for (int c = ncat_min; c < ncat_min+ncat; ++c) {
  // }


  // //************************************************//
  // // Plot jj background fit results per categories 
  // //************************************************//
  // // Plot Background Categories 
  // //****************************//

  // TCanvas* c4 = new TCanvas("c4","jj Background Categories",0,0,2000,2000);
  // c4->Divide(2,1);

  // RooPlot* plotbkg_fit[21];
  // // for (int c = 0; c < ncat; ++c) {
  // for (int c = ncat_min; c < ncat_min+ncat; ++c) {
  //   plotbkg_fit[c] = mgg->frame(Range(minMassFit,maxMassFit),Bins(nBinsMass));
  //   data[c]->plotOn(plotbkg_fit[c],LineColor(kWhite),MarkerColor(kWhite));    
  //   bkg_fit[c]->plotOn(plotbkg_fit[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1)); 
  //   data[c]->plotOn(plotbkg_fit[c]);    
  //   bkg_fit[c]->paramOn(plotbkg_fit[c],Layout(0.4,0.9,0.9), Format("NEU",AutoPrecision(4)));
  //   plotbkg_fit[c]->getAttText()->SetTextSize(0.03);
  //   c4->cd(c+1);
  //   plotbkg_fit[c]->Draw();  
  //   gPad->SetLogy(1);
  //   plotbkg_fit[c]->SetAxisRange(0.1,plotbkg_fit[c]->GetMaximum()*1.5,"Y");
  // }


  // c4->SaveAs((directory+"/plots/backgrounds_log_pretty.pdf").c_str());
  // system(("cp "+directory+"/plots/backgrounds_log_pretty.pdf ~/aQGCVVjj/").c_str());

  // TCanvas* c5 = new TCanvas("c5","jj Background Categories",0,0,2000,2000);
  // c5->Divide(2,1);
  // // c5->Divide(3,7);
  
  // RooPlot* plotbkg_fit2[21];
  // // for (int c = 0; c < ncat; ++c) {
  // for (int c = ncat_min; c < ncat_min+ncat; ++c) {
  //   plotbkg_fit2[c] = mgg->frame(nBinsMass);
  //   data[c]->plotOn(plotbkg_fit[c],LineColor(kWhite),MarkerColor(kWhite));    
  //   bkg_fit[c]->plotOn(plotbkg_fit[c],LineColor(kBlue),Range("fitrange"),NormRange("fitrange"),PrintEvalErrors(-1)); 
  //   data[c]->plotOn(plotbkg_fit[c]);    
  //   bkg_fit[c]->paramOn(plotbkg_fit[c], ShowConstants(true), Layout(0.4,0.9,0.9), Format("NEU",AutoPrecision(4)));
  //   plotbkg_fit[c]->getAttText()->SetTextSize(0.03);
  //   c5->cd(c+1);
  //   plotbkg_fit[c]->Draw();  
  // }

  // c5->SaveAs((directory+"/plots/backgrounds_pretty.pdf").c_str());
  // system(("cp "+directory+"/plots/backgrounds_pretty.pdf ~/aQGCVVjj/").c_str());
}

void MakeSigWS(RooWorkspace* w, const char* fileBaseName, TString signalname, int signalsample, std::vector<string> cat_names, TString cuts) {
  
  TString wsDir   = directory+"/workspaces/"+filePOSTfix;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;
  
  //**********************************************************************//
  // Write pdfs and datasets into the workspace 
  // for statistical tests. 
  //**********************************************************************//

  //********************************//
  // Retrieve P.D.F.s
  //********************************//

  RooAbsPdf* jjSigPdf[21];

  // (1) import signal P.D.F.s

  RooWorkspace *wAll = new RooWorkspace("w_all","w_all");

  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    jjSigPdf[c] = (RooAbsPdf*)  w->pdf(signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str()));
    if(signalsample == 10)wAll->import(*w->pdf(signalname+"_jj"+TString::Format("_sig_%s",cat_names.at(c).c_str())));
    else wAll->import(*w->pdf(signalname+"_jj"+TString::Format("_%s",cat_names.at(c).c_str())));
  }

  cout<< ""<<endl;
  cout<< ""<<endl;
  cout<< "Printing wAll:"<<endl;
  wAll->Print();
  cout<< ""<<endl;
  cout<< ""<<endl;    

  // (2) Systematics on energy scale and resolution put in larges systematic from JES/JER/PDF -> add uncertainties in quadrature

  wAll->factory("CMS_sig_p1_jes_13TeV[0.0,-10.0,10.0]");
  wAll->factory("CMS_jj_sig_p1_jes_13TeV[0.01,0.01,0.01]"); // JES uncertainty
  wAll->factory("sum::CMS_sig_p1_jes_sum_13TeV(1.0,prod::CMS_sig_p1_jes_prod_13TeV(CMS_sig_p1_jes_13TeV, CMS_jj_sig_p1_jes_13TeV))");
  // put systematics for PDF (impact on shape) -> peak

  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    wAll->factory("prod::CMS_jj_"+signalname+"_sig_m0_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_m0_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p1_jes_sum_13TeV)");
    
  }
	
  // (3) Systematics on resolution: create new sigmas


  wAll->factory("CMS_sig_p2_jer_13TeV[0.0,-10.0,10.0]");
  
  wAll->factory("CMS_jj_sig_p2_jer_13TeV[0.1,0.1,0.1]"); // JER uncertainty
  wAll->factory("sum::CMS_sig_p2_jer_sum_13TeV(1.0,prod::CMS_sig_p2_jer_prod_13TeV(CMS_sig_p2_jer_13TeV, CMS_jj_sig_p2_jer_13TeV))");  
 
  // This is for signal != 10 (radion/graviton)
  if(signalsample == 10){
    /*
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
     wAll->factory("prod::CMS_jj_"+signalname+"_sig_sigmaL_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_sigmaL_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
     wAll->factory("prod::CMS_jj_"+signalname+"_sig_sigmaR_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_sigmaR_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
    
  
    wAll->factory("prod::CMS_jj_"+signalname+"_sig_gsigmaL_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_gsigmaL_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
    wAll->factory("prod::CMS_jj_"+signalname+"_sig_gsigmaR_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_gsigmaR_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
  }

    // (4) do reparametrization of signal
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
     std::cout << "print out what happens here : " << std::endl;
     
     std::cout <<"EDIT::"+signalname+"_jj" + TString::Format("_sig_%s(",cat_names.at(c).c_str()) +
      signalname+"_jj" + TString::Format("_%s,",cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_m0_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_m0_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_sigmaL_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigmaL_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_sigmaR_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigmaR_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_gsigmaL_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigmaL_%s_13TeV)", cat_names.at(c).c_str())+
      " jj_"+signalname+TString::Format("_sig_gsigmaR_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigmaR_%s_13TeV)", cat_names.at(c).c_str()) << std::endl;
    wAll->factory(
      "EDIT::"+signalname+"_jj" + TString::Format("_sig_%s(",cat_names.at(c).c_str()) +
      signalname+"_jj" + TString::Format("_%s,",cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_m0_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_m0_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_sigmaL_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigmaL_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_sigmaR_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigmaR_%s_13TeV, ", cat_names.at(c).c_str()) +
      " jj_"+signalname+TString::Format("_sig_gsigmaL_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigmaL_%s_13TeV)", cat_names.at(c).c_str())+
      " jj_"+signalname+TString::Format("_sig_gsigmaR_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigmaR_%s_13TeV)", cat_names.at(c).c_str())
              );
      
  }
*/
// EDIT::ZZ_T0_0p12_jj_sig__invMass(ZZ_T0_0p12_jj__invMass, jj_ZZ_T0_0p12_sig_m0__invMass=CMS_jj_ZZ_T0_0p12_sig_m0__invMass_13TeV,  jj_ZZ_T0_0p12_sig_sigma__invMass=CMS_jj_ZZ_T0_0p12_sig_sigma__invMass_13TeV,  jj_ZZ_T0_0p12_sig_gsigma__invMass=CMS_jj_ZZ_T0_0p12_sig_gsigma__invMass_13TeV)

    
  }else{
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
     wAll->factory("prod::CMS_jj_"+signalname+"_sig_sigma_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_sigma_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
    
  
    wAll->factory("prod::CMS_jj_"+signalname+"_sig_gsigma_"+TString::Format("%s_13TeV",cat_names.at(c).c_str())+"(jj_"+signalname+"_sig_gsigma_"+TString::Format("%s",cat_names.at(c).c_str())+", CMS_sig_p2_jer_sum_13TeV)");
  }

  // (4) do reparametrization of signal
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
     std::cout << "print out what happens here : " << std::endl;
     
     std::cout <<"EDIT::"+signalname+"_jj"+TString::Format("_sig_%s(",cat_names.at(c).c_str())+signalname+"_jj"+TString::Format("_%s,",cat_names.at(c).c_str()) +
        " jj_"+signalname+TString::Format("_sig_m0_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_m0_%s_13TeV, ", cat_names.at(c).c_str()) +
          " jj_"+signalname+TString::Format("_sig_sigma_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigma_%s_13TeV, ", cat_names.at(c).c_str()) +
            " jj_"+signalname+TString::Format("_sig_gsigma_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigma_%s_13TeV)", cat_names.at(c).c_str()) << std::endl;
    wAll->factory(
      "EDIT::"+signalname+"_jj"+TString::Format("_sig_%s(",cat_names.at(c).c_str())+signalname+"_jj"+TString::Format("_%s,",cat_names.at(c).c_str()) +
        " jj_"+signalname+TString::Format("_sig_m0_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_m0_%s_13TeV, ", cat_names.at(c).c_str()) +
          " jj_"+signalname+TString::Format("_sig_sigma_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_sigma_%s_13TeV, ", cat_names.at(c).c_str()) +
            " jj_"+signalname+TString::Format("_sig_gsigma_%s=CMS_jj_",cat_names.at(c).c_str())+signalname+TString::Format("_sig_gsigma_%s_13TeV)", cat_names.at(c).c_str())
              );
  }
  }
  // std::cout<<"------------------"<<std::endl;
  // std::cout<<"------------------"<<std::endl;
  // wAll->var("mgg13TeV")->setVal(1200.);
  // wAll->var("CMS_sig_pdf_13TeV")->setVal(1.);

  // wAll->var("ZZ_T0_1p08_jj_PDFshift__invMass")->printValue();
  // wAll->var("jj_ZZ_T0_1p08_sig_PDFp0__invMass")->printValue();
  // wAll->var("jj_ZZ_T0_1p08_sig_PDFp1__invMass")->printValue();
  // wAll->var("mgg13TeV")->printValue();
  // wAll->var("CMS_sig_pdf_13TeV")->printValue();
  // std::cout<<"------------------"<<std::endl;
  // std::cout<<"------------------"<<std::endl;

  std::cout << " print wAll workspace " << std::endl;
  wAll->Print();
  TString filename(wsDir+TString(fileBaseName)+".root");
  wAll->writeToFile(filename);
  cout << "Write signal workspace in: " << filename << " file" << endl;
  for (int c = ncat_min; c < ncat_min+ncat; ++c){
  std::cout << c<< " "<< cat_names.at(c) << std::endl;
  }
  return;
}


void MakeBkgWS(std::string altfunc, RooWorkspace* w, const char* fileBaseName, std::vector<string> cat_names) {
  
  TString wsDir   = directory+"/workspaces/"+filePOSTfix;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;
  
  //**********************************************************************//
  // Write pdfs and datasets into the workspace 
  // for statistical tests. 
  //**********************************************************************//


  //********************************//
  // Retrieve the datasets and PDFs
  //********************************//

  RooDataSet* data[21];
  RooExtendPdf* bkg_fitPdf[21];

  // (1) import everything

  cout << "Start importing everything" << endl;

  RooWorkspace *wAll = new RooWorkspace("w_all","w_all");

  // for (int c = 0; c < ncat; ++c) {
  for (int c = ncat_min ; c < ncat_min+ncat; ++c) { 
    cout << "For category " << c << endl;
    data[c]      = (RooDataSet*) w->data(TString::Format("Data_%s",cat_names.at(c).c_str()));
    std::cout << data[c] << std::endl;

    int mainCategory=c;
    if (correlateBackgroundParametersBetweenCategories) mainCategory=0;
    
    RooDataHist* dataBinned;
    std::string name = fileBaseName;
    if(name.find("NObinning")!=std::string::npos)
    {
      wAll->import(*data[c], Rename(TString::Format("data_obs_%s",cat_names.at(c).c_str())));
      std::cout << " use no binning " << std::endl;
    }
    if(name.find("binningHCAL")!=std::string::npos)
    {
      double massBins[44] ={1058.,  1118, 1181, 1246, 1313, 1383, 1455, 1530, 1607, 1687, 1770, 1856, 1945, 2037, 2132, 2231, 2332, 2438, 2546, 2659, 2775, 2895, 3019, 3147, 3279, 3416, 3558, 3704, 3854, 4010, 4171, 4337, 4509, 4686, 4869, 5058, 5253, 5455, 5663, 5877, 6099, 6328, 6564, 6808};
      RooBinning mjjbins(43 ,massBins, "mjjbins");
      ((RooRealVar*) data[c]->get()->find("mgg13TeV"))->setBinning(mjjbins);
      dataBinned = data[c]->binnedClone();
      wAll->import(*dataBinned, Rename(TString::Format("data_obs_%s",cat_names.at(c).c_str())));
      std::cout << "use binning of resolution " << std::endl;
    }
    if(name.find("binning")==std::string::npos)
    {    
    ((RooRealVar*) data[c]->get()->find("mgg13TeV"))->setBins(MMAX-MMIN) ;
     dataBinned = data[c]->binnedClone();
     wAll->import(*dataBinned, Rename(TString::Format("data_obs_%s",cat_names.at(c).c_str())));
     std::cout << "use default binning (~1GeV bins) " << std::endl;
    }
    
    //important for me
    if(TString(fileBaseName).Contains("_mjj")){
      bkg_fitPdf[c] = (RooExtendPdf*)  w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()));
      wAll->import(*w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str())));
      wAll->import(*w->function(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())));

      double mean = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getVal();
      double min = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMin();
      double max = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMax();
      wAll->factory(TString::Format("CMS_bkg_fit_%s_13TeV_norm[%g,%g,%g]", cat_names.at(c).c_str(), mean, min, max));

      mean = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getVal();
      min = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMin();
      max = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMax();

      wAll->factory(TString::Format("CMS_bkg_fit_slope1_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));

    }else if (TString(fileBaseName).Contains("_pT"))
    {
     
      bkg_fitPdf[c] = (RooExtendPdf*)  w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()));
      wAll->import(*w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str())));
      wAll->import(*w->function(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())));

      double mean = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getVal();
      double min = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMin();
      double max = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMax();
      wAll->factory(TString::Format("CMS_bkg_fit_%s_13TeV_norm[%g,%g,%g]", cat_names.at(c).c_str(), mean, min, max));

      mean = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getVal();
      min = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMin();
      max = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMax();
      wAll->factory(TString::Format("CMS_bkg_fit_slope1_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));

      mean = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getVal();
      min  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
      max  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
      wAll->factory(TString::Format("CMS_bkg_fit_slope2_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
      
    }
    /*
    
    bkg_fitPdf[c] = (RooExtendPdf*)  w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str()));
    wAll->import(*w->pdf(TString::Format("bkg_fit_%s",cat_names.at(c).c_str())));
    wAll->import(*w->function(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())));

    double mean = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getVal();
    double min = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMin();
    double max = (wAll->var(TString::Format("bkg_fit_%s_norm",cat_names.at(c).c_str())))->getMax();
    wAll->factory(TString::Format("CMS_bkg_fit_%s_13TeV_norm[%g,%g,%g]", cat_names.at(c).c_str(), mean, min, max));

    mean = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getVal();
    min = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMin();
    max = (wAll->var(TString::Format("bkg_fit_slope1_%s",cat_names.at(mainCategory).c_str())))->getMax();

    wAll->factory(TString::Format("CMS_bkg_fit_slope1_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));

    if ( altfunc.find("alt")!=std::string::npos)
    {
       //================ take alt four parameter as fit function  =======================================
       mean = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope2_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
          
        
       mean = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope3_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
        
       mean = (wAll->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope4_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope4_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
    }
    if (altfunc.find("Exp")!=std::string::npos)
    {
        // =============== take levelled exponential as fit function ===================================== 
       mean = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope2_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
        
       mean = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope3_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope3_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max)); 
    }
    if (altfunc.find("3par")!=std::string::npos)
    {
        // =============== take 3 parameter as fit function ===================================== 
       mean = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getVal();
       min  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMin(); 
       max  = (wAll->var(TString::Format("bkg_fit_slope2_%s",cat_names.at(mainCategory).c_str())))->getMax(); 
       wAll->factory(TString::Format("CMS_bkg_fit_slope2_%s_13TeV[%g,%g,%g]", cat_names.at(mainCategory).c_str(), mean, min, max));
        
    }
    */
    cout << "Done For category " << c << endl; 
    wAll->Print();
    
  }
  
  
  cout << "Imported" << endl;

  // (2) do reparametrization of background

  for (int c = ncat_min; c < ncat_min+ncat; ++c) {

    int mainCategory=c;
    if (correlateBackgroundParametersBetweenCategories) mainCategory=0;

    if ( altfunc.find("alt")!=std::string::npos) {
      wAll->factory(
        TString::Format("EDIT::CMS_bkg_fit_%s_13TeV(bkg_fit_%s,",cat_names.at(c).c_str(),cat_names.at(c).c_str()) +
          TString::Format(" bkg_fit_%s_norm=CMS_bkg_fit_%s_13TeV_norm,", cat_names.at(c).c_str(),cat_names.at(c).c_str())+
            TString::Format(" bkg_fit_slope1_%s=CMS_bkg_fit_slope1_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
              TString::Format(" bkg_fit_slope2_%s=CMS_bkg_fit_slope2_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
                TString::Format(" bkg_fit_slope3_%s=CMS_bkg_fit_slope3_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
                  TString::Format(" bkg_fit_slope4_%s=CMS_bkg_fit_slope4_%s_13TeV)", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())
                    );
    }
    if ( altfunc.find("Exp")!=std::string::npos) {
      wAll->factory(
        TString::Format("EDIT::CMS_bkg_fit_%s_13TeV(bkg_fit_%s,",cat_names.at(c).c_str(),cat_names.at(c).c_str()) +
          TString::Format(" bkg_fit_%s_norm=CMS_bkg_fit_%s_13TeV_norm,", cat_names.at(c).c_str(),cat_names.at(c).c_str())+
            TString::Format(" bkg_fit_slope1_%s=CMS_bkg_fit_slope1_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
              TString::Format(" bkg_fit_slope2_%s=CMS_bkg_fit_slope2_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
                TString::Format(" bkg_fit_slope3_%s=CMS_bkg_fit_slope3_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())
                    );
    }
    else if (altfunc.find("3par")!=std::string::npos) {
      wAll->factory(
        TString::Format("EDIT::CMS_bkg_fit_%s_13TeV(bkg_fit_%s,",cat_names.at(c).c_str(),cat_names.at(c).c_str()) +
          TString::Format(" bkg_fit_%s_norm=CMS_bkg_fit_%s_13TeV_norm,", cat_names.at(c).c_str(),cat_names.at(c).c_str())+
            TString::Format(" bkg_fit_slope1_%s=CMS_bkg_fit_slope1_%s_13TeV,", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())+
              TString::Format(" bkg_fit_slope2_%s=CMS_bkg_fit_slope2_%s_13TeV)", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())
                );
    }
    else {
      wAll->factory(
        TString::Format("EDIT::CMS_bkg_fit_%s_13TeV(bkg_fit_%s,",cat_names.at(c).c_str(),cat_names.at(c).c_str()) +
          TString::Format(" bkg_fit_%s_norm=CMS_bkg_fit_%s_13TeV_norm,", cat_names.at(c).c_str(),cat_names.at(c).c_str())+
            TString::Format(" bkg_fit_slope1_%s=CMS_bkg_fit_slope1_%s_13TeV)", cat_names.at(mainCategory).c_str(),cat_names.at(c).c_str())
              );
    }
  }


  TString filename(wsDir+TString(fileBaseName)+".root");
  wAll->writeToFile(filename);
  cout << "Write background workspace in: " << filename << " file" << endl;

  std::cout << "observation ";
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    std::cout << "  " << (wAll->data(TString::Format("data_obs_%s",cat_names.at(c).c_str())))->sumEntries();
    (wAll->data(TString::Format("data_obs_%s",cat_names.at(c).c_str())))->Print();
  }
  std::cout << std::endl;
  
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {

    int mainCategory=c;
    if (correlateBackgroundParametersBetweenCategories) mainCategory=0;

    printf("CMS_bkg_fit_slope1_%s_13TeV  param  %.4f  %.3f   # Mean and absolute uncertainty on background slope\n",
    cat_names.at(c).c_str(), (wAll->var(TString::Format("CMS_bkg_fit_slope1_%s_13TeV",cat_names.at(mainCategory).c_str())))->getVal(), 10.);
  }

  return;
}



void MakeDataCard_1Channel(std::string altfunc, RooWorkspace* w, const char* fileBaseName, const char* fileBkgName, int iChan, TString signalname, int signalsample, std::vector<string> cat_names, double mass) {
  
  TString cardDir = directory+"/datacards/"+filePOSTfix;
  Int_t ncat_min = 0;
  Int_t ncat = NCAT;
  
  TString wsDir   = "../workspaces/"+filePOSTfix;
  

  cout << "Start retrieving dataset" << endl;

  RooDataSet* data[21];
  RooDataSet* signal[21];
  // for (int c = 0; c < NCAT; ++c) {
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    data[c]        = (RooDataSet*) w->data(TString::Format("Data_%s",cat_names.at(c).c_str()));
    signal[c]      = (RooDataSet*) w->data(TString::Format("SigWeight_%s",cat_names.at(c).c_str()));
  }

  //*****************************//
  // Print Expected event yields
  //*****************************//

  cout << "======== Expected Events Number =====================" << endl;  
  cout << "#Events data:        " <<  w->data("Data")->sumEntries()  << endl;
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    cout << "c = " << c << endl;  
    cout << "Name = " << cat_names.at(c).c_str() << endl;  
    cout << TString::Format("#Events data %s:   ",cat_names.at(c).c_str()) << data[c]->sumEntries()  << endl;
  }
  cout << ".........Expected Signal ............................" << endl;  
  cout << "#Events Signal:      " << w->data("SigWeight")->sumEntries()  << endl;
  Float_t siglikeErr[21];
  for (int c = ncat_min; c < ncat_min+ncat; ++c) {
    cout << TString::Format("#Events Signal %s: ",cat_names.at(c).c_str()) << signal[c]->sumEntries() << endl;
    siglikeErr[c]=0.6*signal[c]->sumEntries();
  }
  cout << "====================================================" << endl;  

  //*************************//
  // Print Data Crd int file
  //*************************//

  cout << "Start writing datacard" << endl;

  TString filename(cardDir+TString(fileBaseName)+Form("_%s.txt",cat_names[iChan].c_str()));
  ofstream outFile(filename);

  double scaleFactor=signalScaler;
  
  int mainCategory=iChan;
  if (correlateBackgroundParametersBetweenCategories) mainCategory=0;
  
  cout << "For signalsample " <<signalsample<<"and channel "<<iChan<< endl;
  cout << "scalefactor (before HP/LP SF) = " <<scaleFactor<< endl;
  
  //  HP+HP
  scaleFactor*=(scaleFactorHP*scaleFactorHP);
  
  outFile << "# Fully Hadronic VV analysis" << endl;
  
  outFile << "imax 1" << endl;
  outFile << "jmax 1" << endl;
  outFile << "kmax *" << endl;
  outFile << "---------------" << endl;

  outFile << Form("shapes data_obs   %s ", cat_names[iChan].c_str()) << wsDir+TString(fileBkgName)+".root" << Form(" w_all:data_obs_%s"         , cat_names[iChan].c_str()) << endl;
  outFile << Form("shapes bkg_fit_jj %s ", cat_names[iChan].c_str()) << wsDir+TString(fileBkgName)+".root" << Form(" w_all:CMS_bkg_fit_%s_13TeV", cat_names[iChan].c_str()) << endl;
  
  if(signalsample==0){
    outFile << Form("shapes graviton_jj %s ", cat_names[iChan].c_str()) << wsDir+TString::Format(("CMS_jj_"+altfunc+"graviton_%.0f_13TeV.root").c_str(), mass) << Form(" w_all:graviton_jj_sig_%s", cat_names[iChan].c_str()) << endl;
  } 
  else if(signalsample==1){
    outFile << Form("shapes radion_jj %s ", cat_names[iChan].c_str()) << wsDir+TString::Format(("CMS_jj_"+altfunc+"radion_%.0f_13TeV.root").c_str(), mass ) << Form(" w_all:radion_jj_sig_%s", cat_names[iChan].c_str()) << endl;
  } 
  else if(signalsample==10){
    outFile << Form("shapes %s_jj %s ", signalname.Data(),cat_names[iChan].c_str()) << wsDir+TString::Format(("CMS_jj_"+altfunc+"%s_%.0f_13TeV.root").c_str(), signalname.Data(),mass ) << Form(" w_all:%s_jj_sig_%s", signalname.Data(),cat_names[iChan].c_str()) << endl;
  } 
  
  outFile << "---------------" << endl;
  outFile << Form("bin          %s", cat_names[iChan].c_str()) << endl;
  outFile <<  "observation   "  <<  Form("%.10lg",data[iChan]->sumEntries()) << endl;
  outFile << "------------------------------" << endl;
  
  
    outFile << "bin                     "<< Form("%s       %s      ", cat_names[iChan].c_str(), cat_names[iChan].c_str() )<< endl;
  if(signalsample==0){
    outFile << "process                 graviton_jj     bkg_fit_jj     " << endl;
  }   
  else if(signalsample==1){
    outFile << "process                 radion_jj     bkg_fit_jj     " << endl;
  }
  else if(signalsample==10){
    outFile << "process                 " << signalname << "_jj     bkg_fit_jj     " << endl;
  }
    outFile << "process                 0               1          " << endl;
    outFile << "rate                    "<< signal[iChan]->sumEntries()*scaleFactor<< "         " << 1 << endl;

    cout    << "# signal scaled by " << signalScaler << " to a cross section of 1/pb and also scale factor of " << scaleFactor/signalScaler << " are applied." << endl;
    outFile << "--------------------------------" << endl;
    outFile << "# signal scaled by " << signalScaler << " to a cross section of 1/pb and also scale factor of " << scaleFactor/signalScaler << " are applied." << endl;
 
  // outFile << "CMS_acc_13TeV                       lnN  1.02    - # PDF unc. on acceptance" << endl;
  // outFile << "CMS_pdf_13TeV                       lnN  1.055    - # PDF" << endl;
  // outFile << "CMS_qcd_13TeV                       lnN  1.21    - # QCDScale" << endl;
  outFile << "CMS_pu_13TeV                        lnN  1.02    - # pileup" << endl;
  outFile << "CMS_lumi_13TeV                      lnN  1.026   - # luminosity" << endl;
  outFile << "CMS_jes_13TeV                       lnN  1.06    - # jet energy scale" << endl;
  outFile << "CMS_vtag_13TeV                      lnN  1.12    - # 2xvtagging (tau21<0.4, high pur https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetWtagging#Recipes_to_obtain_the_PUPPI_soft)" << endl;
  
  outFile << "--------------------------------" << endl;
  

  outFile << "# Parametric shape uncertainties, entered by hand." << endl;
  outFile << Form("CMS_sig_p1_jes_13TeV                param   0.0   1.0   # dijet mass shift due to JES uncertainty"           ) << endl;
  // outFile << Form("CMS_sig_p2_jer_13TeV                param   0.0   1.0   # dijet mass resolution shift due to JER uncertainty") << endl;
  // outFile << Form("CMS_sig_pdf_13TeV                   param   0.0   1.0   # signal shape shift due to PDF uncertainty") << endl;
  // outFile << Form("CMS_sig_qcd_13TeV                   param   0.0   1.0   # signal shape shift due to qcd scale uncertainty") << endl;
  
  outFile << Form("CMS_bkg_fit_%s_13TeV_norm           flatParam  # Normalization uncertainty on background slope"    ,cat_names[iChan].c_str()) << endl;

  if(!constrainBackgroundParametersToSimulation)
    outFile << Form("CMS_bkg_fit_slope1_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;  //newStrategy

  std::string name = fileBaseName;
  if (altfunc.find("3par")!=std::string::npos) {
      //======= for 3 parameter fit ================================================
    if(!constrainBackgroundParametersToSimulation)
      outFile << Form("CMS_bkg_fit_slope2_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;  //newStrategy
  } else if (altfunc.find("Exp")!=std::string::npos) {
      //======= for levelled exp ================================================
    if(!constrainBackgroundParametersToSimulation) {
      outFile << Form("CMS_bkg_fit_slope2_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;  //newStrategy
      outFile << Form("CMS_bkg_fit_slope3_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;
    }
  } else if (name.find("alt")!=std::string::npos) {
     //====================== for four parameter function =======================
    if(!constrainBackgroundParametersToSimulation) {
       outFile << Form("CMS_bkg_fit_slope2_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;  //newStrategy
       outFile << Form("CMS_bkg_fit_slope3_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;
       outFile << Form("CMS_bkg_fit_slope4_%s_13TeV         flatParam  # Mean and absolute uncertainty on background slope",cat_names[mainCategory].c_str()) << endl;  //newStrategy
     }
  }
  
  outFile << "--------------------------------" << endl;

  outFile.close();

  cout << "Write data card in: " << filename << " file" << endl;

  return;
}


 Double_t effSigma(TH1 *hist) {

  TAxis *xaxis = hist->GetXaxis();
  Int_t nb = xaxis->GetNbins();
  if(nb < 10) {
    std::cout << "effsigma: Not a valid histo. nbins = " << nb << std::endl;
    return 0.;
  }

  Double_t bwid = xaxis->GetBinWidth(1);
  if(bwid == 0) {
    std::cout << "effsigma: Not a valid histo. bwid = " << bwid << std::endl;
    return 0.;
  }
  Double_t xmax = xaxis->GetXmax();
  Double_t xmin = xaxis->GetXmin();
  Double_t ave = hist->GetMean();
  Double_t rms = hist->GetRMS();

  Double_t total=0.;
  for(Int_t i=0; i<nb+2; i++) {
    total+=hist->GetBinContent(i);
  }
  if(total < 100.) {
    std::cout << "effsigma: Too few entries " << total << std::endl;
    return 0.;
  }
  Int_t ierr=0;
  Int_t ismin=999;

  Double_t rlim=0.683*total;
  Int_t nrms=rms/(bwid);    // Set scan size to +/- rms
  if(nrms > nb/10) nrms=nb/10; // Could be tuned...

  Double_t widmin=9999999.;
  for(Int_t iscan=-nrms;iscan<nrms+1;iscan++) { // Scan window centre
    Int_t ibm=(ave-xmin)/bwid+1+iscan;
    Double_t x=(ibm-0.5)*bwid+xmin;
    Double_t xj=x;
    Double_t xk=x;
    Int_t jbm=ibm;
    Int_t kbm=ibm;
    Double_t bin=hist->GetBinContent(ibm);
    total=bin;
    for(Int_t j=1;j<nb;j++){
      if(jbm < nb) {
        jbm++;
        xj+=bwid;
        bin=hist->GetBinContent(jbm);
        total+=bin;
        if(total > rlim) break;
      }
      else ierr=1;
      if(kbm > 0) {
        kbm--;
        xk-=bwid;
        bin=hist->GetBinContent(kbm);
        total+=bin;
        if(total > rlim) break;
      }
      else ierr=1;
    }
    Double_t dxf=(total-rlim)*bwid/bin;
    Double_t wid=(xj-xk+bwid-dxf)*0.5;
    if(wid < widmin) {
      widmin=wid;
      ismin=iscan;
    }
  }
  if(ismin == nrms || ismin == -nrms) ierr=3;
  if(ierr != 0) std::cout << "effsigma: Error of type " << ierr << std::endl;

  return widmin;
}

void UHHFitter_cuts_newStrategy(string cut, double mass, int signalsamples=0, int channel=0,std::string altfunc="",std::string postfix="")
{
  filePOSTfix=postfix;
  if(TString(cut).Contains("_mjj")){    
    MMIN = 1050.;
    MMAX = 10000.;
    }else if(TString(cut).Contains("_pT")){
    MMIN = 600.;
    MMAX = 4000.;    
  }
  runfits(cut, mass, signalsamples, channel,altfunc);
  std::cout << signalsamples << std::endl;
}
