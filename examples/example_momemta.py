import momemta
import ROOT
LorentzVector = ROOT.Math.LorentzVector(ROOT.Math.PxPyPzE4D("double"))

# Restrict logging to "error" level
momemta.set_log_level(momemta.log_level.error)

momemta_cfg = momemta.ConfigurationReader("WW_fullyleptonic.lua")
momemta_computer = momemta.MoMEMta(momemta_cfg.freeze())


## Using ROOT 4-vectors is also possible
p1 = LorentzVector(10, 20, 30, 200)
p2 = LorentzVector(-10, 20, -30, 200)
print('Type of Lorentz vector')
print(type(p1))

part1 = momemta.Particle("electron", [10, 20, 30 ,200])
part2 = momemta.Particle("muon", [10, 20, 30 ,200])

print('Computing the weight.')
result = momemta_computer.computeWeights([part1, part2])
print("Weight: {} +- {}".format(result[0][0], result[0][1]))
