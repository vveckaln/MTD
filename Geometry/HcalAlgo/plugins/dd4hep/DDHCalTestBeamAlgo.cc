#include "DD4hep/DetFactoryHelper.h"
#include "DataFormats/Math/interface/angle_units.h"
#include "DetectorDescription/DDCMS/interface/DDPlugins.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//#define EDM_ML_DEBUG
using namespace angle_units::operators;
#include "Geometry/HcalAlgo/plugins/dd4hep/HcalDD4HepHelper.h"

static long algorithm(dd4hep::Detector& /* description */, cms::DDParsingContext& ctxt, xml_h e) {
  cms::DDNamespace ns(ctxt, e, true);
  cms::DDAlgoArguments args(ctxt, e);
  // Header section
  double eta = args.value<double>("Eta");                        //Eta at which beam is focussed
  double phi = args.value<double>("Phi");                        //Phi    ................
  double theta = 2.0 * atan(exp(-eta));                          //Corresponding theta value
  double distance = args.value<double>("Dist");                  //Distance of the centre of rotation
  double distanceZ = args.value<double>("DistZ");                //Distance along x-axis of the centre of rotation
  double dist = (distance + distanceZ / sin(theta));             //Overall distance
  int copyNumber = args.value<int>("Number");                    //Copy Number
  std::string childName = args.value<std::string>("ChildName");  //Children name
  double dz = args.value<double>("Dz");                          //Half length along z of the volume to be placed
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HCalGeom") << "DDHCalTestBeamAlgo: Parameters for positioning-- Eta " << eta << "\tPhi "
                               << convertRadToDeg(phi) << "\tTheta " << convertRadToDeg(theta) << "\tDistance "
                               << HcalDD4HepHelper::convert2mm(distance) << "/"
                               << HcalDD4HepHelper::convert2mm(distanceZ) << "/" << HcalDD4HepHelper::convert2mm(dist)
                               << "\tDz " << HcalDD4HepHelper::convert2mm(dz) << "\tcopyNumber " << copyNumber;
  edm::LogVerbatim("HCalGeom") << "DDHCalTestBeamAlgo:Parent " << args.parentName() << "\tChild " << childName
                               << " NameSpace " << ns.name();
#endif

  dd4hep::Volume parent = ns.volume(args.parentName());

  double thetax = 90._deg + theta;
  double sthx = sin(thetax);
  if (std::abs(sthx) > 1.e-12)
    sthx = 1. / sthx;
  else
    sthx = 1.;
  double phix = atan2(sthx * cos(theta) * sin(phi), sthx * cos(theta) * cos(phi));
  double thetay = 90._deg;
  double phiy = 90._deg + phi;
  double thetaz = theta;
  double phiz = phi;

  dd4hep::Rotation3D rotation = cms::makeRotation3D(thetax, phix, thetay, phiy, thetaz, phiz);
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HCalGeom") << "DDHCalTestBeamAlgo: Creating a rotation \t" << convertRadToDeg(thetax) << ","
                               << convertRadToDeg(phix) << "," << convertRadToDeg(thetay) << ","
                               << convertRadToDeg(phiy) << "," << convertRadToDeg(thetaz) << ","
                               << convertRadToDeg(phiz);
#endif

  double r = dist * sin(theta);
  double xpos = r * cos(phi);
  double ypos = r * sin(phi);
  double zpos = dist * cos(theta);
  dd4hep::Position tran(xpos, ypos, zpos);

  childName = ns.prepend(childName);
  dd4hep::Volume child = ns.volume(childName);
  parent.placeVolume(child, copyNumber, dd4hep::Transform3D(rotation, tran));
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HCalGeom") << "DDHCalTestBeamAlgo: " << child.name() << " number " << copyNumber
                               << " positioned in " << parent.name() << " at (" << HcalDD4HepHelper::convert2mm(xpos)
                               << ", " << HcalDD4HepHelper::convert2mm(ypos) << ", "
                               << HcalDD4HepHelper::convert2mm(zpos) << ") with rotation: " << rotation;
#endif
  xpos = (dist - dz) * sin(theta) * cos(phi);
  ypos = (dist - dz) * sin(theta) * sin(phi);
  zpos = (dist - dz) * cos(theta);

  edm::LogInfo("HCalGeom") << "DDHCalTestBeamAlgo: Suggested Beam position (" << HcalDD4HepHelper::convert2mm(xpos)
                           << ", " << HcalDD4HepHelper::convert2mm(ypos) << ", " << HcalDD4HepHelper::convert2mm(zpos)
                           << ") and (dist, eta, phi) = (" << HcalDD4HepHelper::convert2mm(dist - dz) << ", " << eta
                           << ", " << phi << ")";

  return cms::s_executed;
}

// first argument is the type from the xml file
DECLARE_DDCMS_DETELEMENT(DDCMS_hcal_DDHCalTestBeamAlgo, algorithm);
