#include <ped_path_predictor/ped_path_predictor.h>

using namespace std;

/*
Pedestrian Path Predictor Class
*/
PedPathPredictor::PedPathPredictor(){
	world_his = new WorldStateHistroy(nh);

	sim = new RVO::RVOSimulator();
	// Specify global time step of the simulation.
    sim->setTimeStep(0.33f);    
    // Specify default parameters for agents that are subsequently added.
    sim->setAgentDefaults(5.0f, 8, 10.0f, 5.0f, 0.5f, 2.0f);

    //global_frame_id = ModelParams::rosns + "/map";
    global_frame_id = "/map";

    
    ped_prediction_pub = nh.advertise<sensor_msgs::PointCloud>("ped_prediction", 1);

    //AddObstacle();

    timer = nh.createTimer(ros::Duration(1.0/ModelParams::control_freq), &PedPathPredictor::Predict, this);
}

 void PedPathPredictor::AddObstacle(){

 	//TO DO: add obstales to RVO simulator. Get the obstacles info from map_server. Following is just an example

     std::vector<RVO::Vector2> obstacle1, obstacle2, obstacle3, obstacle4;

     // Add (polygonal) obstacle(s), specifying vertices in counterclockwise order.
     float map_width = 20, map_height=15, obs_width = 6, obs_height = 4;
     obstacle1.push_back(RVO::Vector2(0.0f, 0.0f));
     obstacle1.push_back(RVO::Vector2(obs_width, 0.0f));
     obstacle1.push_back(RVO::Vector2(obs_width, obs_height));
     obstacle1.push_back(RVO::Vector2(0.0f, obs_height));

     obstacle2.push_back(RVO::Vector2(map_width-obs_width, 0.0f));
     obstacle2.push_back(RVO::Vector2(map_width, 0.0f));
     obstacle2.push_back(RVO::Vector2(map_width, obs_height));
     obstacle2.push_back(RVO::Vector2(map_width-obs_width, obs_height));

     obstacle3.push_back(RVO::Vector2(map_width-obs_width, map_height-obs_height));
     obstacle3.push_back(RVO::Vector2(map_width, map_height-obs_height));
     obstacle3.push_back(RVO::Vector2(map_width, map_height));
     obstacle3.push_back(RVO::Vector2(map_width-obs_width, map_height));

     obstacle4.push_back(RVO::Vector2(0.0f, map_height-obs_height));
     obstacle4.push_back(RVO::Vector2(obs_width, map_height-obs_height));
     obstacle4.push_back(RVO::Vector2(obs_width, map_height));
     obstacle4.push_back(RVO::Vector2(0.0f, map_height));

     sim->addObstacle(obstacle1);
     sim->addObstacle(obstacle2);
     sim->addObstacle(obstacle3);
     sim->addObstacle(obstacle4);

     /* Process the obstacles so that they are accounted for in the simulation. */
     sim->processObstacles();
}

PedPathPredictor::~PedPathPredictor(){
	if(world_his != NULL) {
		delete world_his;
		world_his = NULL;
	}
	if(sim != NULL) {
		delete sim;
		sim = NULL;
	}
}

void PedPathPredictor::UpdatePed(PedStruct &ped, double cur_pos_x, double cur_pos_y){
    ped.vel_cur.x = (cur_pos_x - ped.pos.x) / ModelParams::RVO2_TIME_PER_STEP;
    ped.vel_cur.y = (cur_pos_y - ped.pos.y) / ModelParams::RVO2_TIME_PER_STEP;
    ped.vel_pref = (ped.vel_cur * (1 - ModelParams::RVO2_PREVEL_WEIGHT)) + (ped.vel_pref * ModelParams::RVO2_PREVEL_WEIGHT);
    ped.pos.x = cur_pos_x;
    ped.pos.y = cur_pos_y;
}

void PedPathPredictor::Predict(const ros::TimerEvent &e){
	
	// Set up the scenario.
	// setupScenario(sim);

	state_cur = world_his->get_current_state();
	sim->clearAllAgents();

	sensor_msgs::PointCloud pc;
	pc.header.frame_id=global_frame_id;
    pc.header.stamp=ros::Time::now();

    //adding pedestrians
    for(int i=0; i<state_cur.peds.size(); i++){
        sim->addAgent(RVO::Vector2(state_cur.peds[i].pos.x, state_cur.peds[i].pos.y));
    }

	// Perform (and manipulate) the simulation.
	for(int step = 0; step < ModelParams::N_SIM_STEP; step++) {
    	for (size_t i = 0; i < state_cur.peds.size(); ++i) {
	        sim->setAgentPrefVelocity(i, RVO::Vector2(state_cur.peds[i].vel_pref.x, state_cur.peds[i].vel_pref.y));
	    }
	    sim->doStep();
	    for (size_t i = 0; i < state_cur.peds.size(); ++i) {
	    	geometry_msgs::Point32 p;
	        p.x = sim->getAgentPosition(i).x();
	        p.y = sim->getAgentPosition(i).y();
	        p.z = 1.0;
	        if(step % 2 == 0) pc.points.push_back(p); // publish ped positions every two time step

	        UpdatePed(state_cur.peds[i], p.x, p.y);
	    }
	}

	ped_prediction_pub.publish(pc);
}