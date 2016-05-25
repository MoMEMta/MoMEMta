/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2016  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <momemta/ConfigurationReader.h>
#include <momemta/MoMEMta.h>
#include <momemta/Utils.h>

#include <iostream>

#include <TH1D.h>
#include <TFile.h>
#include <TTree.h>
#include <TLorentzVector.h>
#include <TCanvas.h>

#include <chrono>

using namespace std::chrono;

int main(int argc, char** argv) {

    UNUSED(argc);
    UNUSED(argv);

    logging::set_level(boost::log::trivial::trace);

    ConfigurationReader configuration("../examples/WW_fullyleptonic.lua");

    MoMEMta weight(configuration.freeze());

    TFile *f = new TFile("/home/fynu/asaggio/scratch/Delphes-3.3.2/new_WW_DelphesCMS.root");
    TTree *tree = (TTree*)f->Get("T");
    //Int_t n = tree->GetEntries();

   
    TLorentzVector* lepton1=0;
    TLorentzVector* lepton2=0;

    //TBranch *b_lepton1;
    //TBranch *b_lepton2;

    tree->SetBranchAddress("lepton1", &lepton1);
    tree->SetBranchAddress("lepton2", &lepton2);
    
    TH1D *h_weights = new TH1D("h_weights", "h_weights", 1000, 0, 1);

    for(Int_t i=0; i<3; i++){
        tree->GetEntry(i);
        
        //std::cout << "LEPTOOOOOOOOOON: " << lepton1->E() << std::endl;

        // lepton1
        LorentzVector p3(lepton1->Px(), lepton1->Py(), lepton1->Pz(), lepton1->E());
        // lepton2
        LorentzVector p4(lepton2->Px(), lepton2->Py(), lepton2->Pz(), lepton2->E());


    // Electron
    //LorentzVector p3(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836);
    // Muo
    //LorentzVector p4(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967);

    auto start_time = system_clock::now();
    std::vector<std::pair<double, double>> weights = weight.computeWeights({p3, p4});
    auto end_time = system_clock::now();

    LOG(debug) << "Result:";
    for (const auto& r: weights) {
        LOG(debug) << r.first << " +- " << r.second;
        h_weights->Fill(r.first);
    }
    
    LOG(info) << "Weight computed in " << std::chrono::duration_cast<milliseconds>(end_time - start_time).count() << "ms";

    }

    new TCanvas();
    h_weights->Draw("");

    return 0;
}
