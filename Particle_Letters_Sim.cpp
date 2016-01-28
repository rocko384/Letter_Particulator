//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010-2011 Alessandro Tasora
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

/*
1.) Add Mesh to letters
2.) Orient letters correctly
3.) Scale letters correctly
4.) Fix control sliders
5.) Fix getting a file
*/

///////////////////////////////////////////////////
//	 CHRONO
//   ------
//   Multibody dinamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include "chrono/physics/ChSystem.h"
#include "chrono/physics/ChBodyEasy.h"
#include "chrono/physics/ChConveyor.h"

#include "chrono_irrlicht/ChIrrApp.h"

#include <iostream>
#include <string>

// Use the namespace of Chrono

using namespace chrono;
using namespace chrono::collision;

// Use the main namespaces of Irrlicht
using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

// Static values valid through the entire program (bad
// programming practice, but enough for quick tests)

double STATIC_flow = 100;
double STATIC_size = .01;
std::vector<ChSharedPtr<ChBody> > particlelist;
std::vector<ChSharedPtr<ChBody> > letterlist;

// Define a MyEventReceiver class which will be used to manage input
// from the GUI graphical user interface

class MyEventReceiver : public IEventReceiver {
public:
	MyEventReceiver(ChIrrAppInterface* myapp) {
		// store pointer applicaiton
		application = myapp;

		// ..add a GUI slider to control particles flow
		scrollbar_flow = application->GetIGUIEnvironment()->addScrollBar(true, rect<s32>(510, 85, 650, 100), 0, 101);
		scrollbar_flow->setMax(300);
		scrollbar_flow->setPos(150);
		text_flow = application->GetIGUIEnvironment()->addStaticText(L"Flow [particles/s]",
			rect<s32>(650, 85, 750, 100), false);

		// ..add GUI slider to particle size
		scrollbar_speed = application->GetIGUIEnvironment()->addScrollBar(true, rect<s32>(510, 125, 650, 140), 0, 102);
		scrollbar_speed->setMax(1);
		scrollbar_speed->setPos(.1);
		text_speed = application->GetIGUIEnvironment()->addStaticText(L"Particle Size [m]",
			rect<s32>(650, 125, 750, 140), false);
	}

	bool OnEvent(const SEvent& event) {
		// check if user moved the sliders with mouse..
		if (event.EventType == EET_GUI_EVENT) {
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = application->GetIGUIEnvironment();

			switch (event.GUIEvent.EventType) {
			case EGET_SCROLL_BAR_CHANGED:
				if (id == 101)  // id of 'flow' slider..
				{
					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					STATIC_flow = (double)pos;
				}
				if (id == 102)  // id of 'size' slider..
				{
					s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					STATIC_size = ((double)pos);
				}
				break;
			}
		}

		return false;
	}

private:
	ChIrrAppInterface* application;

	IGUIScrollBar* scrollbar_flow;
	IGUIStaticText* text_flow;
	IGUIScrollBar* scrollbar_speed;
	IGUIStaticText* text_speed;
};

// Function that creates debris that fall on the conveyor belt, to be called at each dt

void create_debris(ChIrrApp& application, double dt, double particles_second, std::string letters) {
	double xnozzlesize = .5;
	double znozzlesize = .08*letters.length();
	double ynozzle = .8;

	double density = 3;
	double sphrad = STATIC_size;
	double sphmass = (4 / 3) * CH_C_PI * pow(sphrad, 3) * density;
	double sphinertia = pow(sphrad, 2) * sphmass;

	double exact_particles_dt = dt * particles_second;
	double particles_dt = floor(exact_particles_dt);
	double remaind = exact_particles_dt - particles_dt;
	if (remaind > ChRandom())
		particles_dt += 1;

	video::ITexture* bluwhiteMap = application.GetVideoDriver()->getTexture(GetChronoDataFile("bluwhite.png").c_str());
	
	for (int i = 0; i < particles_dt; i++) {
		double rand_fract = ChRandom();

			ChSharedPtr<ChBodyEasySphere> mrigidBody(new ChBodyEasySphere(sphrad,  // size
				3,       // density
				true,    // collide enable?
				true));  // visualization?
			mrigidBody->SetPos(ChVector<>(-0.5 * xnozzlesize + ChRandom() * xnozzlesize, ynozzle + i * 0.005 + .25,
				-0.5 * znozzlesize + ChRandom() * znozzlesize));
			mrigidBody->GetMaterialSurface()->SetFriction(0.2f);
			mrigidBody->GetMaterialSurface()->SetRestitution(0.8f);
			mrigidBody->AddAsset(ChSharedPtr<ChTexture>(new ChTexture(GetChronoDataFile("bluwhite.png"))));

			application.GetSystem()->Add(mrigidBody);

			// This will make particle's visualization assets visible in Irrlicht:
			application.AssetBind(mrigidBody);
			application.AssetUpdate(mrigidBody);

			particlelist.push_back(mrigidBody);
		
	}

}

// Function that deletes old debris (to avoid infinite creation that fills memory)

void purge_debris(ChIrrAppInterface& application, int nmaxparticles = 1000) {
	while (particlelist.size() > nmaxparticles) {
		application.GetSystem()->Remove(particlelist[0]);  // remove from physical simulation
		particlelist.erase(particlelist.begin());  // remove also from our particle list (will also automatically delete
												   // object thank to shared pointer)
	}
}

void assemble_letters(ChIrrApp& application, std::string letters) {
	for (int i = 0; i < letters.length(); i++) {
		ChSharedPtr<ChBody> letterBody(new ChBody());
		ChSharedPtr<ChObjShapeFile> lettermesh(new ChObjShapeFile);
		ChSharedPtr<ChObjShapeFile> lettertexture(new ChObjShapeFile);

		std::string str;
		str = letters;

		lettermesh->SetFilename(GetChronoDataFile((letters[i] + ".obj")));
		std::cout << lettermesh->GetFilename() << std::endl;
		lettertexture->SetFilename(GetChronoDataFile("RedTexture.png"));

		std::cout << "NNNNNNNNNNNNN" << std::endl;
		std::cout << letters[i] + ".obj" << std::endl;
		//std::cout << ".png" << std::endl;
		std::cout << "NNNNNNNNNNNNN" << std::endl;

		/*
		std::cout << "AAAAAAAA" << std::endl;
		std::cout << letters << std::endl;
		std::cout << "AAAAAAAA" << std::endl;
		*/

		letterBody->AddAsset(lettermesh);
		letterBody->SetBodyFixed(true);
		letterBody->RecomputeCollisionModel();
		letterBody->SetCollide(true);
		letterBody->SetPos(ChVector<>( 0, .5, 0));
		letterBody->SetRot(Q_from_AngAxis(90, ChVector<>((3.1415926/2), 0, 0)));
		letterBody->AddAsset(lettertexture);

		application.GetSystem()->Add(letterBody);

		application.AssetBind(letterBody);
		application.AssetUpdate(letterBody);

		letterlist.push_back(letterBody);


	};
}

int main(int argc, char* argv[]) {
	//Take in characters to simulate
	std::cout << "Please input letters" << std::endl;
	std::string letters;
	std::cin >> letters;


	// Create a ChronoENGINE physical system
	ChSystem mphysicalSystem;

	// Create the Irrlicht visualization (open the Irrlicht device,
	// bind a simple user interface, etc. etc.)
	ChIrrApp application(&mphysicalSystem, L"Conveyor belt", core::dimension2d<u32>(800, 600), false);

	// Easy shortcuts to add camera, lights, logo and sky in Irrlicht scene:
	ChIrrWizard::add_typical_Logo(application.GetDevice());
	ChIrrWizard::add_typical_Sky(application.GetDevice());
	ChIrrWizard::add_typical_Lights(application.GetDevice(), core::vector3df(-1.5, .25, 0),
		core::vector3df((f32)0, 0, 0));
	ChIrrWizard::add_typical_Camera(application.GetDevice(), core::vector3df(-.6 * letters.length(), 0, 0),
		core::vector3df((f32)0, 0, 0));

	// This is for GUI tweaking of system parameters..
	MyEventReceiver receiver(&application);
	// note how to add the custom event receiver to the default interface:
	application.SetUserEventReceiver(&receiver);

	// Set small collision envelopes for objects that will be created from now on..
	ChCollisionModel::SetDefaultSuggestedEnvelope(0.002);
	ChCollisionModel::SetDefaultSuggestedMargin(0.002);

	// Create the five walls of the rectangular container, using
	// fixed rigid bodies of 'box' type:
	// X - Depth
	// Y - Height
	// Z - Width

	ChSharedPtr<ChBodyEasyBox> floorBody(new ChBodyEasyBox( 1, .1, .5*letters.length(), 1000, true, true));
	floorBody->SetPos(ChVector<>(0, 0, 0));
	floorBody->SetBodyFixed(true);

	application.GetSystem()->Add(floorBody);
	

	ChSharedPtr<ChBodyEasyBox> wallBody1(new ChBodyEasyBox(.1, 1, .5*letters.length(), 1000, true, false));
	wallBody1->SetPos(ChVector<>(-.5, .5, 0));
	wallBody1->SetBodyFixed(true);

	application.GetSystem()->Add(wallBody1);

	ChSharedPtr<ChBodyEasyBox> wallBody2(new ChBodyEasyBox(.1, 1, .5*letters.length(), 1000, true, false));
	wallBody2->SetPos(ChVector<>(.5, .5, 0));
	wallBody2->SetBodyFixed(true);

	application.GetSystem()->Add(wallBody2);

	ChSharedPtr<ChBodyEasyBox> wallBody3(new ChBodyEasyBox(1, 1, .1, 1000, true, false));
	wallBody3->SetPos(ChVector<>(0, .5, .25*letters.length()));
	wallBody3->SetBodyFixed(true);

	application.GetSystem()->Add(wallBody3);

	ChSharedPtr<ChBodyEasyBox> wallBody4(new ChBodyEasyBox(1, 1, .1, 1000, true, false));
	wallBody4->SetPos(ChVector<>(0, .5, -.25*letters.length()));
	wallBody4->SetBodyFixed(true);

	application.GetSystem()->Add(wallBody4);

	// optional, attach  textures for better visualization
	ChSharedPtr<ChTexture> mtexturewall(new ChTexture());
	mtexturewall->SetTextureFilename(GetChronoDataFile("concrete.jpg"));
	wallBody1->AddAsset(mtexturewall);  // note: most assets can be shared
	wallBody2->AddAsset(mtexturewall);
	wallBody3->AddAsset(mtexturewall);
	wallBody4->AddAsset(mtexturewall);
	floorBody->AddAsset(mtexturewall);

	assemble_letters(application, letters);

	// Create an Irrlicht 'directory' where debris will be put during the simulation loop
	ISceneNode* parent = application.GetSceneManager()->addEmptySceneNode();

	//
	// THE SOFT-REAL-TIME CYCLE
	//
	application.AssetBindAll();
	application.AssetUpdateAll();

	application.SetStepManage(true);
	application.SetTimestep(0.005);

	while (application.GetDevice()->run()) {
		application.GetVideoDriver()->beginScene(true, true, SColor(255, 140, 161, 192));

		application.DrawAll();

		application.DoStep();

		if (!application.GetPaused()) {
			// Continuosly create debris that fall on the conveyor belt
			create_debris(application, application.GetTimestep(), STATIC_flow, letters);

			// Limit the max number of debris particles on the scene, deleting the oldest ones, for performance
			purge_debris(application, 300);

		}

		application.GetVideoDriver()->endScene();
	}

	return 0;
}