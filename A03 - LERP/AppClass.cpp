#include "AppClass.h"
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Alberto Bobadilla - labigm@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUpward(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides

	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		float theta = 2 * PI / (float)i;
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		float radius = fSize - 0.1f / 2;
		fSize += 0.5f; //increment the size for the next orbit
		std::vector<vector3> stop_points;
		for (uint j = 0; j < i; j++) {
			stop_points.push_back(vector3(radius*cos(theta*j), radius*sin(theta*j), 0));
		}
		m_stop_points_vectors.push_back(stop_points);
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}

	for (size_t i = 0; i < m_uOrbits; i++)
	{
		std::vector<vector3> m_stopsList = m_stop_points_vectors.at(i);
		m_currStopIndex_vec.push_back(0);
		m_v3CurrentPos_vec.push_back(m_stopsList.at(0));
		m_prevPos_vec.push_back(m_stopsList.at(0));
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 1.5708f, AXIS_Z);

	static float fTimer = 0;	//store the new timer
	static uint uClock = m_pSystem->GenClock(); //generate a new clock for that timer
	static float percentage = 0;
	const float max = 0.5f;
	fTimer += m_pSystem->GetDeltaTime(uClock); //get the delta time for that timer

	// Ensure we don't go too far with timer
	if (fTimer > max)
	{
		fTimer -= max;
		percentage = 1.0f;
	}
	else
	{
		percentage = MapValue(fTimer, 0.0f, max, 0.0f, 1.0f);
	}

	// Start drawing shapes and circles
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 1.5708f, AXIS_X));

		//Grab info needed for this iteration (previous stop and curr position)
		vector3 v3CurrentPos = m_v3CurrentPos_vec.at(i);
		vector3 prevPos = m_prevPos_vec.at(i);
		int currStopIndex = m_currStopIndex_vec.at(i);				
		std::vector<vector3> m_stopsList = m_stop_points_vectors.at(i);

		// Check if we've hit next stop. If so, swap values accordingly.
		if ((v3CurrentPos - m_stopsList.at(currStopIndex)).length() < 0.1f || percentage >= 1.0f)
		{
			percentage = 1.0f;
			v3CurrentPos = m_stopsList.at(currStopIndex);
			prevPos = m_stopsList.at(currStopIndex);
			currStopIndex++;
			currStopIndex %= m_stopsList.size();
		}
		else
		{
			// Otherwise, lerp for the correct percentage
			v3CurrentPos = glm::lerp(prevPos, m_stopsList.at(currStopIndex), percentage);
		}

		// Store back everything from this iteration
		m_currStopIndex_vec.at(i) = currStopIndex;
		m_v3CurrentPos_vec.at(i) = v3CurrentPos;
		m_prevPos_vec.at(i) = prevPos;

		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}