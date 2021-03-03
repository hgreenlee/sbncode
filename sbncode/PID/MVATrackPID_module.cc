////////////////////////////////////////////////////////////////////////
// Class:       MVATrackPID
// Plugin Type: producer (art v3_05_01)
// File:        MVATrackPID_module.cc
//
// Generated at Tue Jan 26 08:37:49 2021 by Edward Tyley using cetskelgen
// from cetlib version v3_10_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art_root_io/TFileService.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "lardata/DetectorInfoServices/DetectorClocksService.h"
#include "lardataobj/AnalysisBase/Calorimetry.h"
#include "lardataobj/AnalysisBase/ParticleID.h"
#include "lardataobj/RecoBase/Cluster.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/MCSFitResult.h"
#include "lardataobj/RecoBase/PFParticle.h"
#include "lardataobj/RecoBase/PFParticleMetadata.h"
#include "lardataobj/RecoBase/Track.h"
#include "larsim/MCCheater/BackTrackerService.h"
#include "larsim/MCCheater/ParticleInventoryService.h"
#include "larsim/Utils/TruthMatchUtils.h"

#include "sbnobj/Common/Reco/LGCFit.h"
#include "sbnobj/Common/Reco/MVAPID.h"
#include "sbnobj/Common/Reco/RangeP.h"
#include "sbnobj/Common/Reco/ScatterDCA.h"
#include "sbnobj/Common/Reco/StoppingChi2Fit.h"

#include "sbncode/LArRecoProducer/LArReco/LGfitter.h"

// Root Includes
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TMath.h"
#include "TTree.h"

#include "TMVA/MethodCuts.h"
#include "TMVA/Reader.h"
#include "TMVA/Tools.h"

#include <iostream>
#include <vector>

namespace sbn {
class MVATrackPID;
}

class sbn::MVATrackPID : public art::EDProducer {
  public:
  explicit MVATrackPID(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MVATrackPID(MVATrackPID const&) = delete;
  MVATrackPID(MVATrackPID&&) = delete;
  MVATrackPID& operator=(MVATrackPID const&) = delete;
  MVATrackPID& operator=(MVATrackPID&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;
  void beginJob() override;

  private:
  // Declare member data here.
  art::ServiceHandle<art::TFileService> tfs;
  art::ServiceHandle<cheat::ParticleInventoryService> particleInventory;

  art::InputTag fLArGeantLabel, fPFPLabel, fTrackLabel, fCaloLabel, fMCSLabel, fChi2Label, fRangeLabel, fLGCLabel, fDCALabel, fStoppingChi2Label;
  const float fMinTrackLength;
  const bool fMakeTree, fRunMVA;
  const std::string fMethodName, fWeightFile;

  const LGfitter::LGfitter lgFitter;

  // The metrics actually used in the MVA
  float recoLen;                                 // The length of the track
  float chi2PIDMuon, chi2PIDPion, chi2PIDProton; // Chi2 PID scores for different hyptheses
  float mcsScatterMean, mscScatterMax;           // Scattering angles used in MCS calculation
  float meanDCA;                                 // Distacnce of closest approach to interpolated track
  float stoppingChi2Ratio;                       // Ratio of exp/pol0 chi2 fits to end of track
  float lgcMPV;                                  // Most Probable value of fitted Landau-Guassian
  float pDiff;                                   // Relatve momentum agreement between range and MCS
  //TODO: Root insists that we pass the reader floats, but these should really be ints:w
  float numDaughters, maxDaughterHits; // Hierarchy of the track

  // MVA scores
  float muonScore, pionScore, protonScore, otherScore, bestScore;
  int bestPDG;

  // Other metrics for filling in tree for analysis
  TMVA::Reader* reader;
  TTree* trackTree;

  int truePdg, chi2PIDPDG, chi2PIDPDGNoKaon, numHits, bestPlane, bestPlaneHits, hierarchyDepth, trueStopping, recoContained, recoPrimary;

  float chi2PIDKaon;
  float chi2Pol0Fit, chi2Pol0Chi2, chi2ExpChi2;
  float mcsScatterMax;
  float lgcAmp, lgcGaussWidth, lgcLandauWidth, lgcChi2;
  float stdDevDCA, maxDCA;
  float mcsMuonP, rangeMuonP;
  float trackScore, daughterTrackScore;
  float startX, startY, startZ, endX, endY, endZ, trueStartX, trueStartY, trueStartZ, trueEndX, trueEndY, trueEndZ, startDist, endDist, trueP, energyComp, energyPurity;

  std::string trueType, trueEndProcess, chi2PIDType, chi2PIDTypeNoKaon;

  void ClearTree();
  void FillPFPMetrics(const art::Ptr<recob::PFParticle>& pfp, const std::map<size_t, art::Ptr<recob::PFParticle>>& pfpMap,
      const art::FindManyP<recob::Cluster>& fmCluster, const art::FindManyP<recob::Hit>& fmHit, const art::FindManyP<larpandoraobj::PFParticleMetadata>& fmMeta);
  void FillTrueParticleMetrics(const detinfo::DetectorClocksData& clockData, const recob::Track& track, const std::vector<art::Ptr<recob::Hit>>& hits,
      std::vector<art::Ptr<sim::SimChannel>>& simChannels);
  void FillTrackMetrics(const recob::Track& track);
  void FillMCSMetrics(const recob::MCSFitResult& mcs);
  void FillChi2PIDMetrics(const anab::ParticleID& pid);
  void FillRangePMetrics(const sbn::RangeP& range);
  void FillLGCMetrics(const sbn::LGCFit& lgc);
  void FillDCAMetrics(const sbn::ScatterDCA& dca);
  void FillStoppingChi2Metrics(const sbn::StoppingChi2Fit& stoppingChi2);
  sbn::MVAPID RunMVA();

  std::map<size_t, art::Ptr<recob::PFParticle>> GetPFPMap(std::vector<art::Ptr<recob::PFParticle>>& pfps) const;
  unsigned int GetPFPHierarchyDepth(const art::Ptr<recob::PFParticle>& pfp, const std::map<size_t, art::Ptr<recob::PFParticle>>& pfpMap) const;
  float GetPFPTrackScore(const art::Ptr<recob::PFParticle>& pfp, const art::FindManyP<larpandoraobj::PFParticleMetadata>& fmMeta) const;
  bool InFV(const TVector3& pos) const;
  std::string PdgString(const int pdg) const;
};

sbn::MVATrackPID::MVATrackPID(fhicl::ParameterSet const& p)
    : EDProducer { p }
    , fLArGeantLabel(p.get<std::string>("LArGeantLabel"))
    , fPFPLabel(p.get<std::string>("PFPLabel"))
    , fTrackLabel(p.get<std::string>("TrackLabel"))
    , fCaloLabel(p.get<std::string>("CaloLabel"))
    , fMCSLabel(p.get<std::string>("MCSLabel"), std::string("muon"))
    , fChi2Label(p.get<std::string>("Chi2Label"))
    , fRangeLabel(p.get<std::string>("RangeLabel"), std::string("muon"))
    , fLGCLabel(p.get<std::string>("LGCLabel"))
    , fDCALabel(p.get<std::string>("DCALabel"))
    , fStoppingChi2Label(p.get<std::string>("StoppingChi2Label"))
    , fMinTrackLength(p.get<float>("MinTrackLength"))
    , fMakeTree(p.get<bool>("MakeTree"))
    , fRunMVA(p.get<bool>("RunMVA"))
    , fMethodName(p.get<std::string>("MethodName", ""))
    , fWeightFile(p.get<std::string>("WeightFile", ""))
    , lgFitter(p.get<float>("LGFitterNP", 100.f), p.get<float>("LGFitterSC", 8.f))
{
  if (!fMakeTree && !fRunMVA)
    throw cet::exception("MVATrackPID") << "Configured to do nothing";

  if (fRunMVA) {
    if (fMethodName == "" || fWeightFile == "")
      throw cet::exception("MVATrackPID") << "Trying to run MVA with inputs not set: MethodName: " << fMethodName << " and WeightFile: " << fWeightFile;

    cet::search_path searchPath("FW_SEARCH_PATH");
    std::string fWeightFileFullPath;
    if (!searchPath.find_file(fWeightFile, fWeightFileFullPath))
      throw cet::exception("MVATrackPID") << "Unable to find weight file: " << fWeightFile << " in FW_SEARCH_PATH: " << searchPath.to_string();

    // reader = new TMVA::Reader("Error");
    reader = new TMVA::Reader("V");

    // std::cout << "Adding Variable: recoLen" << std::endl;
    reader->AddVariable("recoLen", &recoLen);

    reader->AddVariable("chi2PIDMuon", &chi2PIDMuon);
    reader->AddVariable("chi2PIDProton", &chi2PIDProton);
    reader->AddVariable("chi2PIDPion", &chi2PIDPion);

    reader->AddVariable("mcsScatterMean", &mcsScatterMean);
    reader->AddVariable("mcsScatterMax", &mcsScatterMax);
    reader->AddVariable("meanDCA", &meanDCA);

    reader->AddVariable("stoppingChi2Ratio", &stoppingChi2Ratio);
    reader->AddVariable("lgcMPV", &lgcMPV);

    reader->AddVariable("pDiff", &pDiff);
    reader->AddVariable("numDaughters", &numDaughters);
    reader->AddVariable("maxDaughterHits", &maxDaughterHits);

    reader->BookMVA(fMethodName, fWeightFileFullPath);
  }
  // Call appropriate produces<>() functions here.
  // produces<std::vector<sbn::MVAPID>>();
  // produces<art::Assns<recob::Track, sbn::MVAPID>>();
}

void sbn::MVATrackPID::beginJob()
{
  if (fMakeTree) {
    trackTree = tfs->make<TTree>("trackTree", "Tree filled per Track with  PID variables");

    // The metrics actually used in the MVA
    trackTree->Branch("recoLen", &recoLen);
    trackTree->Branch("chi2PIDMuon", &chi2PIDMuon);
    trackTree->Branch("chi2PIDPion", &chi2PIDPion);
    trackTree->Branch("chi2PIDProton", &chi2PIDProton);
    trackTree->Branch("mcsScatterMean", &mcsScatterMean);
    trackTree->Branch("mcsScatterMax", &mcsScatterMax);
    trackTree->Branch("meanDCA", &meanDCA);
    trackTree->Branch("stoppingChi2Ratio", &stoppingChi2Ratio);
    trackTree->Branch("lgcMPV", &lgcMPV);
    trackTree->Branch("pDiff", &pDiff);
    trackTree->Branch("numDaughters", &numDaughters);
    trackTree->Branch("maxDaughterHits", &maxDaughterHits);

    if (fRunMVA) {
      trackTree->Branch("muonScore", &muonScore);
      trackTree->Branch("pionScore", &pionScore);
      trackTree->Branch("protonScore", &protonScore);
      trackTree->Branch("otherScore", &otherScore);
      trackTree->Branch("bestScore", &bestScore);
      trackTree->Branch("bestPDG", &bestPDG);
    }

    trackTree->Branch("truePdg", &truePdg);
    trackTree->Branch("trueP", &trueP);
    trackTree->Branch("trueType", &trueType);
    trackTree->Branch("trueEndProcess", &trueEndProcess);
    trackTree->Branch("energyComp", &energyComp);
    trackTree->Branch("energyPurity", &energyPurity);

    trackTree->Branch("trueStopping", &trueStopping);

    trackTree->Branch("startX", &startX);
    trackTree->Branch("startY", &startY);
    trackTree->Branch("startZ", &startZ);

    trackTree->Branch("endX", &endX);
    trackTree->Branch("endY", &endY);
    trackTree->Branch("endZ", &endZ);

    trackTree->Branch("trueStartX", &trueStartX);
    trackTree->Branch("trueStartY", &trueStartY);
    trackTree->Branch("trueStartZ", &trueStartZ);

    trackTree->Branch("trueEndX", &trueEndX);
    trackTree->Branch("trueEndY", &trueEndY);
    trackTree->Branch("trueEndZ", &trueEndZ);

    trackTree->Branch("startDist", &startDist);
    trackTree->Branch("endDist", &endDist);

    trackTree->Branch("numHits", &numHits);
    trackTree->Branch("bestPlane", &bestPlane);
    trackTree->Branch("bestPlaneHits", &bestPlaneHits);
    trackTree->Branch("hierarchyDepth", &hierarchyDepth);
    trackTree->Branch("trackScore", &trackScore);
    trackTree->Branch("daughterTrackScore", &daughterTrackScore);

    trackTree->Branch("recoPrimary", &recoPrimary);
    trackTree->Branch("recoContained", &recoContained);

    trackTree->Branch("chi2PIDKaon", &chi2PIDKaon);
    trackTree->Branch("chi2PIDPDG", &chi2PIDPDG);
    trackTree->Branch("chi2PIDPDGNoKaon", &chi2PIDPDGNoKaon);
    trackTree->Branch("chi2PIDType", &chi2PIDType);
    trackTree->Branch("chi2PIDTypeNoKaon", &chi2PIDTypeNoKaon);

    trackTree->Branch("lgcAmp", &lgcAmp);
    trackTree->Branch("lgcGaussWidth", &lgcGaussWidth);
    trackTree->Branch("lgcLandauWidth", &lgcLandauWidth);
    trackTree->Branch("lgcChi2", &lgcChi2);

    trackTree->Branch("chi2Pol0Fit", &chi2Pol0Fit);
    trackTree->Branch("chi2Pol0Chi2", &chi2Pol0Chi2);
    trackTree->Branch("chi2ExpChi2", &chi2ExpChi2);

    trackTree->Branch("mcsMuonP", &mcsMuonP);

    trackTree->Branch("rangeMuonP", &rangeMuonP);

    trackTree->Branch("stdDevDCA", &stdDevDCA);
    trackTree->Branch("maxDCA", &maxDCA);
  }
}

void sbn::MVATrackPID::produce(art::Event& e)
{
  auto const clockData(art::ServiceHandle<detinfo::DetectorClocksService const>()->DataFor(e));

  auto const pfpHandle(e.getValidHandle<std::vector<recob::PFParticle>>(fPFPLabel));
  auto const clusterHandle(e.getValidHandle<std::vector<recob::Cluster>>(fPFPLabel));
  auto const simChannelHandle(e.getValidHandle<std::vector<sim::SimChannel>>(fLArGeantLabel));
  auto const trackHandle(e.getValidHandle<std::vector<recob::Track>>(fTrackLabel));

  std::vector<art::Ptr<recob::PFParticle>> pfps;
  art::fill_ptr_vector(pfps, pfpHandle);

  std::vector<art::Ptr<sim::SimChannel>> simChannels;
  art::fill_ptr_vector(simChannels, simChannelHandle);

  art::FindManyP<recob::Cluster> fmPFPCluster(pfpHandle, e, fPFPLabel);
  art::FindManyP<larpandoraobj::PFParticleMetadata> fmPFPMeta(pfpHandle, e, fPFPLabel);
  art::FindManyP<recob::Hit> fmClusterHit(clusterHandle, e, fPFPLabel);

  art::FindManyP<recob::Track> fmPFPTrack(pfpHandle, e, fTrackLabel);
  art::FindManyP<recob::Hit> fmTrackHit(trackHandle, e, fTrackLabel);
  art::FindManyP<anab::Calorimetry> fmTrackCalo(trackHandle, e, fCaloLabel);
  art::FindManyP<recob::MCSFitResult> fmTrackMCS(trackHandle, e, fMCSLabel);
  art::FindManyP<anab::ParticleID> fmTrackChi2(trackHandle, e, fChi2Label);

  art::FindManyP<sbn::LGCFit> fmTrackLGC(trackHandle, e, fLGCLabel);
  art::FindManyP<sbn::RangeP> fmTrackRange(trackHandle, e, fRangeLabel);
  art::FindManyP<sbn::ScatterDCA> fmTrackDCA(trackHandle, e, fDCALabel);
  art::FindManyP<sbn::StoppingChi2Fit> fmTrackStoppingChi2(trackHandle, e, fStoppingChi2Label);

  const std::map<size_t, art::Ptr<recob::PFParticle>> pfpMap(this->GetPFPMap(pfps));

  for (auto const& pfp : pfps) {
    this->ClearTree();

    // Get the track for the PFP
    std::vector<art::Ptr<recob::Track>> pfpTrackVec(fmPFPTrack.at(pfp.key()));

    // Skip showers and primaries
    if (pfpTrackVec.empty())
      continue;

    // There should be 1-1 PFP to tracks
    if (pfpTrackVec.size() > 1)
      throw cet::exception("MVATrackPID") << "Too many tracks: " << pfpTrackVec.size();

    art::Ptr<recob::Track>& pfpTrack(pfpTrackVec.front());

    if (pfpTrack->Length() < fMinTrackLength)
      continue;

    this->FillTrackMetrics(*pfpTrack);

    // TODO look at in terms of runtime
    if (!recoContained)
      continue;

    // We expect a calo for each plane
    auto const caloVec(fmTrackCalo.at(pfpTrack.key()));
    if (caloVec.size() != 3)
      continue;

    // Fill only for the best plane, defined as the one with the most hits
    // Prefer collection plane > 1st induction > 2nd induction
    const unsigned int maxHits(std::max(caloVec[0]->dEdx().size(), std::max(caloVec[1]->dEdx().size(), caloVec[2]->dEdx().size())));
    bestPlane = (caloVec[2]->dEdx().size() == maxHits) ? 2 : (caloVec[0]->dEdx().size() == maxHits) ? 0 : (caloVec[1]->dEdx().size() == maxHits) ? 1 : -1;
    bestPlaneHits = maxHits;

    if (bestPlane < 0 || bestPlane > 3)
      throw cet::exception("MVATrackPID") << "Best plane: " << bestPlane;

    this->FillPFPMetrics(pfp, pfpMap, fmPFPCluster, fmClusterHit, fmPFPMeta);

    // We should have a MCS fit result for muon, proton, pion and kaon hypotheses
    // For now, just consider the muon
    auto const mcsVec(fmTrackMCS.at(pfpTrack.key()));
    if (mcsVec.size() == 1)
      this->FillMCSMetrics(*mcsVec.front());

    // We should have a rangeP fit for muon and proton
    // For now, just consider the muon
    auto const rangeVec(fmTrackRange.at(pfpTrack.key()));
    if (rangeVec.size() == 1)
      this->FillRangePMetrics(*rangeVec.front());

    // We expect a chi2 for each plane
    auto const chi2Vec(fmTrackChi2.at(pfpTrack.key()));
    if (chi2Vec.size() == 3)
      this->FillChi2PIDMetrics(*chi2Vec[bestPlane]);

    auto const lgcVec(fmTrackLGC.at(pfpTrack.key()));
    if (lgcVec.size() == 1)
      this->FillLGCMetrics(*lgcVec.front());

    auto const dcaVec(fmTrackDCA.at(pfpTrack.key()));
    if (dcaVec.size() == 1)
      this->FillDCAMetrics(*dcaVec.front());

    auto const stoppingChi2Vec(fmTrackStoppingChi2.at(pfpTrack.key()));
    if (stoppingChi2Vec.size() == 1)
      this->FillStoppingChi2Metrics(*stoppingChi2Vec.front());

    if (fRunMVA)
      this->RunMVA();

    // Only fill the truth metrics if we are saving a TTree
    if (fMakeTree) {
      std::vector<art::Ptr<recob::Hit>> trackHitVec(fmTrackHit.at(pfpTrack.key()));
      numHits = trackHitVec.size();
      this->FillTrueParticleMetrics(clockData, *pfpTrack, trackHitVec, simChannels);
      trackTree->Fill();
    }
  }
}

void sbn::MVATrackPID::ClearTree()
{
  truePdg = -5;
  chi2PIDPDG = -5;
  chi2PIDPDGNoKaon = -5;
  numHits = -5;
  bestPlane = -5;
  numDaughters = -5;
  hierarchyDepth = -5;
  trackScore = -5;
  maxDaughterHits = -5;
  daughterTrackScore = -5;

  trueStopping = -5;
  recoContained = -5;
  recoPrimary = -5;

  muonScore = -5.f;
  pionScore = -5.f;
  protonScore = -5.f;
  otherScore = -5.f;
  bestScore = -5.f;
  bestPDG = -5;

  startX = -999.f;
  startY = -999.f;
  startZ = -999.f;
  endX = -999.f;
  endY = -999.f;
  endZ = -999.f;
  trueStartX = -999.f;
  trueStartY = -999.f;
  trueStartZ = -999.f;
  trueEndX = -999.f;
  trueEndY = -999.f;
  trueEndZ = -999.f;
  startDist = -999.f;
  endDist = -999.f;

  trueP = -5.f;
  energyPurity = -5.f;
  energyComp = -5.f;
  recoLen = -5.f;
  mcsMuonP = -5.f;
  rangeMuonP = -5.f;
  pDiff = -5.f;
  chi2PIDMuon = -5.f;
  chi2PIDPion = -5.f;
  chi2PIDKaon = -5.f;
  chi2PIDProton = -5.f;
  lgcMPV = -5.f;
  lgcAmp = -5.f;
  lgcGaussWidth = -5.f;
  lgcLandauWidth = -5.f;
  lgcChi2 = -5.f;
  chi2Pol0Fit = -5.f;
  chi2Pol0Chi2 = -5.f;
  chi2ExpChi2 = -5.f;
  stoppingChi2Ratio = -5.f;
  mcsScatterMean = -5.f;
  mcsScatterMax = -5.f;
  meanDCA = -5.f;
  stdDevDCA = -5.f;
  maxDCA = -5.f;

  trueType = "";
  trueEndProcess = "";
  chi2PIDType = "";
  chi2PIDTypeNoKaon = "";
}

void sbn::MVATrackPID::FillPFPMetrics(const art::Ptr<recob::PFParticle>& pfp, const std::map<size_t, art::Ptr<recob::PFParticle>>& pfpMap,
    const art::FindManyP<recob::Cluster>& fmCluster, const art::FindManyP<recob::Hit>& fmHit, const art::FindManyP<larpandoraobj::PFParticleMetadata>& fmMeta)
{
  try {
    recoPrimary = (int)pfpMap.at(pfp->Parent())->IsPrimary();
  } catch (...) {
  }
  numDaughters = pfp->Daughters().size();
  trackScore = this->GetPFPTrackScore(pfp, fmMeta);

  if (!numDaughters)
    return;

  for (auto const daughterId : pfp->Daughters()) {
    auto const& daughter(pfpMap.at(daughterId));
    auto const& clusters(fmCluster.at(daughter.key()));
    int daughterHits(0);
    for (auto const& cluster : clusters) {
      daughterHits += fmHit.at(cluster.key()).size();
    }
    if (daughterHits > maxDaughterHits) {
      maxDaughterHits = daughterHits;
      daughterTrackScore = this->GetPFPTrackScore(daughter, fmMeta);
    }
  }

  if (!fMakeTree)
    return;

  hierarchyDepth = this->GetPFPHierarchyDepth(pfp, pfpMap);
}

void sbn::MVATrackPID::FillTrueParticleMetrics(const detinfo::DetectorClocksData& clockData, const recob::Track& track, const std::vector<art::Ptr<recob::Hit>>& hits, std::vector<art::Ptr<sim::SimChannel>>& simChannels)
{
  art::ServiceHandle<cheat::BackTrackerService> bt_serv;
  const int bestMatch(TruthMatchUtils::TrueParticleIDFromTotalTrueEnergy(clockData, hits, true));

  if (!TruthMatchUtils::Valid(bestMatch))
    return;

  float totalHitEnergy(0.f), totalTrueHitEnergy(0.f);
  for (auto const& hit : hits) {
    const std::vector<sim::TrackIDE> trackIDEs(bt_serv->HitToTrackIDEs(clockData, hit));
    totalHitEnergy = std::accumulate(trackIDEs.cbegin(), trackIDEs.cend(), totalHitEnergy,
        [](float sum, auto const& ide) { return sum + ide.energy; });
    totalTrueHitEnergy = std::accumulate(trackIDEs.cbegin(), trackIDEs.cend(), totalTrueHitEnergy,
        [bestMatch](float sum, auto const& ide) { return (std::abs(ide.trackID) == bestMatch) ? sum + ide.energy : sum; });
  }

  const std::vector<const sim::IDE*> trackIDEs(bt_serv->TrackIdToSimIDEs_Ps(bestMatch));
  float totalTrueEnergy(std::accumulate(trackIDEs.cbegin(), trackIDEs.cend(), 0.f,
      [](float sum, auto const& ide) { return sum + ide->energy; }));

  energyComp = totalTrueHitEnergy / totalTrueEnergy;
  energyPurity = totalTrueHitEnergy / totalHitEnergy;

  const simb::MCParticle* const trueParticle(particleInventory->TrackIdToParticle_P(bestMatch));
  truePdg = trueParticle->PdgCode();
  trueType = this->PdgString(truePdg);
  trueEndProcess = trueParticle->EndProcess();

  trueStopping = (int)(trueEndProcess == "CoupledTransportation" || trueEndProcess == "FastScintillation" || trueEndProcess == "Decay" || trueEndProcess == "muMinusCaptureAtRest");

  trueP = trueParticle->P();

  const TVector3 trackStart(track.Start<TVector3>());
  const TVector3 trackEnd(track.End<TVector3>());

  const TVector3 trueStart(trueParticle->Position().Vect());
  const TVector3 trueEnd(trueParticle->EndPosition().Vect());

  trueStartX = trueStart.X();
  trueStartY = trueStart.Y();
  trueStartZ = trueStart.Z();

  trueEndX = trueEnd.X();
  trueEndY = trueEnd.Y();
  trueEndZ = trueEnd.Z();

  startDist = (trueStart - trackStart).Mag();
  endDist = (trueEnd - trackEnd).Mag();
}

void sbn::MVATrackPID::FillTrackMetrics(const recob::Track& track)
{
  recoLen = track.Length();

  const TVector3 start(track.Start<TVector3>());
  const TVector3 end(track.End<TVector3>());
  recoContained = this->InFV(start) && this->InFV(end);

  if (!fMakeTree)
    return;

  startX = start.X();
  startY = start.Y();
  startZ = start.Z();

  endX = end.X();
  endY = end.Y();
  endZ = end.Z();
}

bool sbn::MVATrackPID::InFV(const TVector3& pos) const
{
  return (std::abs(pos.X()) < 195 && std::abs(pos.Y()) < 195 && pos.Z() > 5 && pos.Z() < 495);
}

void sbn::MVATrackPID::FillMCSMetrics(const recob::MCSFitResult& mcs)
{
  mcsMuonP = mcs.fwdMomentum();

  if (mcs.scatterAngles().empty())
    return;

  unsigned int counter(0);
  float maxScatter(0), meanScatter(0);
  for (auto const& angle : mcs.scatterAngles()) {
    if (angle < 0)
      continue;
    maxScatter = std::max(maxScatter, angle);
    meanScatter += angle;
    counter++;
  }

  if (!counter)
    return;

  mcsScatterMax = maxScatter;
  mcsScatterMean = meanScatter / counter;
}

void sbn::MVATrackPID::FillRangePMetrics(const sbn::RangeP& range)
{
  rangeMuonP = range.range_p;

  pDiff = (rangeMuonP > 0 && mcsMuonP > 0) ? (mcsMuonP - rangeMuonP) / rangeMuonP : -5.f;
}

void sbn::MVATrackPID::FillLGCMetrics(const sbn::LGCFit& lgc)
{
  lgcMPV = lgc.mMPV;

  if (!fMakeTree)
    return;

  lgcAmp = lgc.mAmplitude;
  lgcGaussWidth = lgc.mGaussWidth;
  lgcLandauWidth = lgc.mLandauWidth;
  lgcChi2 = lgc.mChi2 / lgc.mNDF;
}

void sbn::MVATrackPID::FillDCAMetrics(const sbn::ScatterDCA& dca)
{
  meanDCA = dca.mMean;

  if (!fMakeTree)
    return;

  stdDevDCA = dca.mStdDev;
  maxDCA = dca.mMax;
}

void sbn::MVATrackPID::FillStoppingChi2Metrics(const sbn::StoppingChi2Fit& stoppingChi2)
{
  chi2Pol0Chi2 = stoppingChi2.mPol0Chi2;
  chi2ExpChi2 = stoppingChi2.mExpChi2;

  stoppingChi2Ratio = (chi2Pol0Chi2 > 0.f && chi2ExpChi2 > 0.f) ? chi2Pol0Chi2 / chi2ExpChi2 : -5.f;

  if (!fMakeTree)
    return;

  chi2Pol0Fit = stoppingChi2.mPol0Fit;
}

sbn::MVAPID sbn::MVATrackPID::RunMVA()
{
  const std::vector<float> mvaScores(reader->EvaluateMulticlass(fMethodName));

  sbn::MVAPID pidResults;

  pidResults.AddScore(13, mvaScores.at(0));
  pidResults.AddScore(211, mvaScores.at(1));
  pidResults.AddScore(2212, mvaScores.at(2));
  pidResults.AddScore(0, mvaScores.at(3));

  if (!fMakeTree)
    return pidResults;

  muonScore = mvaScores.at(0);
  pionScore = mvaScores.at(1);
  protonScore = mvaScores.at(2);
  otherScore = mvaScores.at(3);

  bestScore = pidResults.BestScore();
  bestPDG = pidResults.BestPDG();

  return pidResults;
}

void sbn::MVATrackPID::FillChi2PIDMetrics(const anab::ParticleID& pid)
{
  chi2PIDMuon = pid.Chi2Muon();
  chi2PIDPion = pid.Chi2Pion();
  chi2PIDKaon = pid.Chi2Kaon();
  chi2PIDProton = pid.Chi2Proton();

  if (!fMakeTree)
    return;

  chi2PIDPDG = pid.Pdg();
  chi2PIDType = this->PdgString(pid.Pdg());

  if (chi2PIDPDG != 321) {
    chi2PIDPDGNoKaon = chi2PIDPDG;
    chi2PIDTypeNoKaon = chi2PIDType;
  } else {
    if (chi2PIDMuon < chi2PIDProton && chi2PIDMuon < chi2PIDPion) {
      chi2PIDPDGNoKaon = 13;
    } else if (chi2PIDPion < chi2PIDProton) {
      chi2PIDPDGNoKaon = 211;
    } else {
      chi2PIDPDGNoKaon = 2212;
    }
    chi2PIDTypeNoKaon = this->PdgString(chi2PIDPDGNoKaon);
  }
}

std::map<size_t, art::Ptr<recob::PFParticle>> sbn::MVATrackPID::GetPFPMap(std::vector<art::Ptr<recob::PFParticle>>& pfps) const
{
  std::map<size_t, art::Ptr<recob::PFParticle>> pfpMap;
  for (auto const& pfp : pfps) {
    pfpMap[pfp->Self()] = pfp;
  }
  return pfpMap;
}

unsigned int sbn::MVATrackPID::GetPFPHierarchyDepth(const art::Ptr<recob::PFParticle>& pfp, const std::map<size_t, art::Ptr<recob::PFParticle>>& pfpMap) const
{
  if (pfp->Daughters().empty()) {
    return 1;
  }
  unsigned int maxDepth(0);
  for (auto const daughter : pfp->Daughters()) {
    maxDepth = std::max(maxDepth, this->GetPFPHierarchyDepth(pfpMap.at(daughter), pfpMap));
  }
  return maxDepth + 1;
}

float sbn::MVATrackPID::GetPFPTrackScore(const art::Ptr<recob::PFParticle>& pfp, const art::FindManyP<larpandoraobj::PFParticleMetadata>& fmMeta) const
{
  auto const pfpMetaVec(fmMeta.at(pfp.key()));
  if (pfpMetaVec.size() != 1)
    return -5.f;
  const larpandoraobj::PFParticleMetadata::PropertiesMap propertiesMap(pfpMetaVec.front()->GetPropertiesMap());
  auto const& pfpTrackScoreIter = propertiesMap.find("TrackScore");
  return pfpTrackScoreIter == propertiesMap.end() ? -5.f : pfpTrackScoreIter->second;
}

std::string sbn::MVATrackPID::PdgString(const int pdg) const
{
  switch (std::abs(pdg)) {
  case 13:
    return "Mu";
  case 211:
    return "Pion";
  case 321:
    return "Kaon";
  case 2212:
    return "Proton";
  default:
    return "Other";
  }
}

DEFINE_ART_MODULE(sbn::MVATrackPID)
