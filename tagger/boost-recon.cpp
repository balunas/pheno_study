/*! Boosted Analysis */
/*!
 *Two FatJets with pt>200GeV and |eta|< 2.0
 *From the two FatJets we reconstruct the Higgs
 *which must fall in |m_h-125|< 40.
 */

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
#include <TH1I.h>
#include <TROOT.h>
#include <TTree.h>

#include <range/v3/all.hpp>

#include "classes/DelphesClasses.h"
#include "classes/DelphesFactory.h"
#include "modules/Delphes.h"

#include "Cutflow.h"
#include "utils.h"

using namespace ROOT;
namespace view = ranges::view;
namespace action = ranges::action;

constexpr double GeV = 1.; ///< Set to 1 -- energies and momenta in GeV

bool two_large_b_jets(
    VecOps::RVec<Jet> &jets /*!Input Jets of FastJet <Jet> type*/) {
  /**
  *Filter for events which
  *respect Boosted Category
  *Requirements
  *If events is boosted return true
  *and it passes the selection
  */
  int count = 0;   // Counting Number of Boosted Events
  int b_count = 0; // Counting Number of Boosted B-tagged Events

  for (auto &&j : jets) {
    if (j.PT >= 20. * GeV and std::abs(j.Eta) < 2.5) {
      count++;
      if (j.BTag)
        b_count++;
    }
    if (count > 2 and b_count > 2)
      return true; // Return Type Boolean
  }
}

bool tagging(
    VecOps::RVec<Jet> &fatjets, // Input  FatJet of FastJet <Jet> type
    VecOps::RVec<Jet> &trkjets,
    VecOps::RVec<HepMCEvent> &evt // Input Event of Delphes <HepMCEvent> type
    ) {
  /**
  *Event Reconstruction Function
  *Store FastJet Jets in <OxJet> type
  *Filter them for Large-R jets
  *Take the top two in pT
  *Reconstruct the Higgs from leading and subleading Jets
  */
  // Ghost association and BTagging
  std::cout << "Starting Tagging Algorithm" << std::endl;

  std::vector<TaggedJet> tagged_vec;
  for (auto calojet : fatjets) {
    TaggedJet tagged_jet;
    get_assoc_trkjets(calojet, trkjets, tagged_jet);
    tagged_vec.push_back(tagged_jet);
  }

  int b_count = 0;

  for (auto tag : tagged_vec) {
    std::cout << "Tagged NTAG" << tag.ntag << std::endl;
    if (tag.ntag > 2)
      b_count++;
  }

  std::cout << "B Counted" << b_count << std::endl;
  if (b_count > 2)
    return true;

  return false;
}

bool valid_check(const reconstructed_event &evt) {
  return evt.valid;
} // Filter Only Valid Events

bool signal(const reconstructed_event &evt) {
  /*
  *Filter reconstructed events
  *whose Higgs bosons
  *fall in the signal Mass window
  */

  double m_h1 = evt.higgs1.p4.M(); // Mass of Leading Higgs
  double m_h2 = evt.higgs2.p4.M(); // Mass of Subleading Higgs
  // Cut in a mass window of 80 GeV around 125 GeV for both Higgs
  bool higgs1_flag = (std::abs(evt.higgs1.p4.M() - 125.) < 40.) ? true : false;
  bool higgs2_flag = (std::abs(evt.higgs2.p4.M() - 125.) < 40.) ? true : false;
  return (higgs1_flag && higgs2_flag); // Return Type boolean: if both Higgs
                                       // fall in the window then accept
}

bool control(const reconstructed_event &evt) {
  /*
  *Filter reconstructed events
  *whose Higgs bosons
  *fall in the control Mass window
  */

  double m_h1 = evt.higgs1.p4.M(); // Mass of Leading Higgs
  double m_h2 = evt.higgs2.p4.M(); // Mass of subleading Higgs

  // Cut in a mass window of 80 GeV around 125 GeV for both Higgs
  bool higgs1_flag = (std::abs(evt.higgs1.p4.M() - 125.) < 45.) ? true : false;
  bool higgs2_flag = (std::abs(evt.higgs2.p4.M() - 125.) < 45.) ? true : false;
  return (higgs1_flag && higgs2_flag); // Return Type boolean: if both Higgs
                                       // fall in the window then accept
}

bool sideband(const reconstructed_event &evt) {

  /*
  *Filter reconstructed events
  *whose Higgs bosons
  *fall in the sideband Mass window
  */

  double m_h1 = evt.higgs1.p4.M(); // Mass of the Leading Higgs
  double m_h2 = evt.higgs2.p4.M(); // Mass of the Subleading Higgs

  // Cut in a mass window of 80 GeV around 125 GeV for both Higgs
  bool higgs1_flag = (std::abs(evt.higgs1.p4.M() - 125.) < 50.) ? true : false;
  bool higgs2_flag = (std::abs(evt.higgs2.p4.M() - 125.) < 50.) ? true : false;
  return (higgs1_flag && higgs2_flag); // Return Type boolean: if both Higgs
                                       // fall in the window then accept
}

std::vector<std::string>
files(const std::string &path // Path to the input files
      ,
      const int &nfiles // Number of input files
      ,
      const std::string &tag) {
  /**
   * Function to import the input file
   * The file should be in the same folder
   * such that the input path is the same
   */

  std::vector<std::string>
      file_names; // Vector including all files paths and addresses
  for (int i = 0; i < nfiles; i++) {
    file_names.push_back(fmt::format("{}/{}_{}.root", path, tag, i));
  }
  return file_names; // Returns the vector with files to be read by RDF
}


int main(int arc, char *argv[]) {

  std::ios::sync_with_stdio(false);
  using vec_string = std::vector<std::string>;

  ROOT::EnableImplicitMT();

  // const std::string file_path = argv[1];
  // const std::string file_tag = argv[2];
  // const int file_numb= atoi(argv[3]);

  // RDataFrame frame("Delphes", files(file_path,file_numb,file_tag)); //Input
  // file for RDF

  RDataFrame frame("Delphes", "/data/atlas/atlasdata/micheli/validation/"
                              "delphes/signal_jesse/signal_jesse.root");
  // auto two_b_jets = frame.Filter(two_large_b_jets,{"FatJet"},u8"Resolved
  // analysis cuts"); //Apply Boosted Filter
  // frame.Foreach([] { static int i = 0; std::cout << i++ << std::endl; });
  auto reconstructed = frame.Filter(tagging, {"FatJet", "TrackJet", "Event"},
                                    "FatBtagging"); // Reconstruct Events
  std::cout << "After Reconstructed" << std::endl;
  std::cout << "Reconstructed Count: " << reconstructed.Count().GetValue()
            << std::endl;

  std::cout << "Stored Reconstructed" << std::endl;
  /*
  auto valid_evt = reconstructed.Filter(valid_check,{"event"},"valid events");
  //Filter only valid Events

  auto signal_result = valid_evt.Filter(signal,{"event"},"signal"); //Filter
  Signal Events
  auto control_result = valid_evt.Filter(
        [](const reconstructed_event &event) {
          return control(event) && (!signal(event));
        },
        {"event"}, "control"); // Filter Events in the Control Region
  auto sideband_result = valid_evt.Filter(
        [](const reconstructed_event &event) {
          return sideband(event) && !control(event);
        },
        {"event"}, "sideband"); //Filter Events int the Sideband Region

  std::string output_filename = "pheno_boosted.root"; //Name Output File

  fmt::print("Writing to {}\n", output_filename);

  auto start_events_proxy = frame.Count();
  start_events_proxy.OnPartialResult(
        10000, [](const unsigned long long &num_events) {
        fmt::print("Processed {} events\n", num_events);
        });

  TFile output_file(output_filename.c_str(), "RECREATE"); //Opening Ouput File
  write_tree(signal_result, "signal", output_file); //Writing the Signal Tree
  write_tree(control_result, "control", output_file); //Writing the Control Tree
  write_tree(sideband_result, "sideband", output_file); //Writing the Sideband
  Tree


  start_events_proxy.GetValue();

  Cutflow boosted_cutflow("Intermediate Cutflow", output_file); //Define Cutflow
  for the Boosted Analysis
          boosted_cutflow.add(u8"2 small good jets(pT ≥ 40 GeV, η ≤ 2.5), ≥ 2
  tagged, 1 large good jet",
                        two_b_jets.Count());
          boosted_cutflow.add(u8"Reconstructed events",
                        reconstructed.Count());
          boosted_cutflow.add(u8"1 large jet  and 2 small jets Tagged",
                        valid_evt.Count());
          boosted_cutflow.add(u8"Signal",
                        signal_result.Count());
          boosted_cutflow.add(u8"Signal",
                        control_result.Count());
          boosted_cutflow.add(u8"Signal",
                        sideband_result.Count());
          boosted_cutflow.write();

  */

  return 0;
}
