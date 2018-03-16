#! /bin/env python

import argparse
import momemta
import unittest
import sys

CONFIGURATION_FILE = ''

class IntegrationTest(unittest.TestCase):
    def setUp(self):
        global CONFIGURATION_FILE

        momemta.set_log_level(momemta.log_level.error)
        self.reader = momemta.ConfigurationReader(CONFIGURATION_FILE)

        # Change number of iterations for cuba
        cuba = self.reader.getCubaConfiguration()
        cuba.setInt("verbosity", 0)
        cuba.setInt("max_eval", 2000)
        cuba.setInt("n_start", 500)
        cuba.setDouble("relative_accuracy", 0.5)

        self.runner = momemta.MoMEMta(self.reader.freeze())

    def test_pyroot(self):
        import ROOT
        LorentzVector = ROOT.Math.LorentzVector('ROOT::Math::PxPyPzE4D<double>')

        # List of LorentzVector
        p3 = momemta.Particle("electron", LorentzVector(16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836), 0)
        p4 = momemta.Particle("bjet1", LorentzVector(-55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625), 0)
        p5 = momemta.Particle("muon", LorentzVector(-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967), 0)
        p6 = momemta.Particle("bjet2", LorentzVector(71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352), 0)

        result = self.runner.computeWeights([p3, p4, p5, p6])
        self.assertEqual(self.runner.getIntegrationStatus(), momemta.IntegrationStatus.SUCCESS)

        self.assertEqual(len(result), 1)
        result = result[0]
        self.assertAlmostEqual(result[0], 4.4954322e-21)
        self.assertAlmostEqual(result[1], 2.1120765e-21)

        self.runner.setEvent([p3, p4, p5, p6])
        result = self.runner.evaluateIntegrand([0.5]*9)
        self.assertEqual(len(result), 1)
        self.assertAlmostEqual(result[0], 2.06407675147e-18)

    def test_list(self):

        # Plain list as LorentzVector
        p3 = momemta.Particle("electron", [16.171895980835, -13.7919054031372, -3.42997527122497, 21.5293197631836], 0)
        p4 = momemta.Particle("bjet1", [-55.7908325195313, -111.59294128418, -122.144721984863, 174.66259765625], 0)
        p5 = momemta.Particle("muon", [-18.9018573760986, 10.0896110534668, -0.602926552295686, 21.4346446990967], 0)
        p6 = momemta.Particle("bjet2", [71.3899612426758, 96.0094833374023, -77.2513122558594, 142.492813110352], 0)

        result = self.runner.computeWeights([p3, p4, p5, p6])
        self.assertEqual(self.runner.getIntegrationStatus(), momemta.IntegrationStatus.SUCCESS)

        self.assertEqual(len(result), 1)
        result = result[0]
        self.assertAlmostEqual(result[0], 4.4954322e-21)
        self.assertAlmostEqual(result[1], 2.1120765e-21)

        self.runner.setEvent([p3, p4, p5, p6])
        result = self.runner.evaluateIntegrand([0.5]*9)
        self.assertEqual(len(result), 1)
        self.assertAlmostEqual(result[0], 2.06407675147e-18)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('configuration', type=str)

    options, args = parser.parse_known_args()

    CONFIGURATION_FILE = options.configuration

    unittest.main(argv=sys.argv[:1] + args)
