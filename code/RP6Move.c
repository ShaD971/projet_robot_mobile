

/*****************************************************************************/
// Includes:

#include "RP6RobotBaseLib.h" 	

/*****************************************************************************/
// Behaviour command type:

#define IDLE  0
#define TOTO 2


// commande du robot:
typedef struct {
	uint8_t  speed_left;  //vitesse moteur gauche
	uint8_t  speed_right; // vitesse moteur droit
	unsigned dir:2;       // direction (FWD, BWD, LEFT, RIGHT)
	unsigned move:1;      // move flag
	unsigned rotate:1;    // rotate flag
	uint16_t move_value;  // la valeur de déplacement est utilisé pour les valeurs de distance et d'angle
	uint8_t  state;       // état du comportement
} behaviour_command_t;

behaviour_command_t STOP = {0, 0, FWD, false, false, 0, IDLE};

/*****************************************************************************/
// Comportement normal:

#define CRUISE_SPEED_FWD    100 // 100 la vitesse par défaut lorsque aucun obstacle sont détectés!

#define MOVE_FORWARDS 1
behaviour_command_t cruise = {CRUISE_SPEED_FWD, CRUISE_SPEED_FWD, FWD, 
								false, false, 0, MOVE_FORWARDS};
//comportement normal
void behaviour_cruise(void)
{
}

/*****************************************************************************/
// Comportement apres detection d'obstacle:

#define ESCAPE_SPEED_BWD    40 // 100
#define ESCAPE_SPEED_ROTATE 30  // 60

#define ESCAPE_FRONT		1
#define ESCAPE_FRONT_WAIT 	2
#define ESCAPE_LEFT  		3
#define ESCAPE_LEFT_WAIT	4
#define ESCAPE_RIGHT	    5
#define ESCAPE_RIGHT_WAIT 	6
#define ESCAPE_WAIT_END		7
behaviour_command_t escape = {0, 0, FWD, false, false, 0, IDLE}; 


/**
 *  le comportement d'échappement pour les Bumpers.
 */
void behaviour_escape(void)
{
	static uint8_t bump_count = 0;
	
	switch(escape.state)
	{
		case IDLE: 
		break;
		case ESCAPE_FRONT:
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir = BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 220;
			else
				escape.move_value = 160;
			escape.state = ESCAPE_FRONT_WAIT;
			bump_count+=2;
		break;
		case ESCAPE_FRONT_WAIT:			
			if(!escape.move) // Attend que le mouvement soit complet
			{	
				escape.speed_left = ESCAPE_SPEED_ROTATE;
				if(bump_count > 3)
				{
					escape.move_value = 100;
					escape.dir = RIGHT;
					bump_count = 0;
				}
				else 
				{
					escape.dir = LEFT;
					escape.move_value = 70;
				}
				escape.rotate = true;
				escape.state = ESCAPE_WAIT_END;
			}
		break;
		case ESCAPE_LEFT:
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir 	= BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 190;
			else
				escape.move_value = 150;
			escape.state = ESCAPE_LEFT_WAIT;
			bump_count++;
		break;
		case ESCAPE_LEFT_WAIT:
			if(!escape.move) // Attend que le mouvement soit complet
			{
				escape.speed_left = ESCAPE_SPEED_ROTATE;
				escape.dir = RIGHT;
				escape.rotate = true;
				if(bump_count > 3)
				{
					escape.move_value = 110;
					bump_count = 0;
				}
				else{
					escape.move_value = 80;
				escape.state = ESCAPE_WAIT_END;
			}
			}
		break;
		case ESCAPE_RIGHT:	
			escape.speed_left = ESCAPE_SPEED_BWD;
			escape.dir 	= BWD;
			escape.move = true;
			if(bump_count > 3)
				escape.move_value = 190;
			else
				escape.move_value = 150;
			escape.state = ESCAPE_RIGHT_WAIT;
			bump_count++;
		break;
		case ESCAPE_RIGHT_WAIT:
			if(!escape.move) // Attend que le mouvement soit complet
			{ 
				escape.speed_left = ESCAPE_SPEED_ROTATE;		
				escape.dir = LEFT;
				escape.rotate = true;
				if(bump_count > 3)
				{
					escape.move_value = 110;
					bump_count = 0;
				}
				else
					escape.move_value = 80;
				escape.state = ESCAPE_WAIT_END;
			}
		break;
		case ESCAPE_WAIT_END:
			if(!(escape.move || escape.rotate)) // Attend que le mouvement ou la rotation soit complet
				escape.state = IDLE;
		break;
	}
}

/**
 * Bumpers Event handler
 */
void bumpersStateChanged(void)
{
	if(bumper_left && bumper_right) // Les deux bumper
	{
		escape.state = ESCAPE_FRONT;
	}
	else if(bumper_left)  			// bumper gauche
	{
		if(escape.state != ESCAPE_FRONT_WAIT) 
			escape.state = ESCAPE_LEFT;
	}
	else if(bumper_right) {			// bumper droit
		if(escape.state != ESCAPE_FRONT_WAIT)
			escape.state = ESCAPE_RIGHT;
	}
}

/*****************************************************************************/
// comportement avec infrarouge:

// Valeur de vitesse:
#define AVOID_SPEED_L_ARC_LEFT  30
#define AVOID_SPEED_L_ARC_RIGHT 40 // 90
#define AVOID_SPEED_R_ARC_LEFT  40 // 90
#define AVOID_SPEED_R_ARC_RIGHT 30
#define AVOID_SPEED_ROTATE 	30     // 60

// status du comportement:
#define AVOID_OBSTACLE_RIGHT 		1
#define AVOID_OBSTACLE_LEFT 		2
#define AVOID_OBSTACLE_MIDDLE	    3
#define AVOID_OBSTACLE_MIDDLE_WAIT 	4
#define AVOID_END 					5
behaviour_command_t avoid = {0, 0, FWD, false, false, 0, IDLE};

/**
 * comportement avec les infrarouges
 */
void behaviour_avoid(void)
{
	static uint8_t last_obstacle = LEFT;
	static uint8_t obstacle_counter = 0;
	switch(avoid.state)
	{
		case IDLE: 
		
			if(obstacle_right && obstacle_left) // capteur droit et gauche detecte
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			else if(obstacle_left)  // capteur gauche detecte
				avoid.state = AVOID_OBSTACLE_LEFT;
			else if(obstacle_right) // capteur droit detecte
				avoid.state = AVOID_OBSTACLE_RIGHT;
		break;
		case AVOID_OBSTACLE_MIDDLE:
			avoid.dir = last_obstacle;
			avoid.speed_left = AVOID_SPEED_ROTATE;
			avoid.speed_right = AVOID_SPEED_ROTATE;
			if(!(obstacle_left || obstacle_right))
			{
				if(obstacle_counter > 3)
				{
					obstacle_counter = 0;
					setStopwatch4(0);
				}
				else
					setStopwatch4(400);
				startStopwatch4();
				avoid.state = AVOID_END;
			}
		break;
		case AVOID_OBSTACLE_RIGHT:
			avoid.dir = FWD;
			avoid.speed_left = AVOID_SPEED_L_ARC_LEFT;
			avoid.speed_right = AVOID_SPEED_L_ARC_RIGHT;
			if(obstacle_right && obstacle_left)
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			if(!obstacle_right)
			{
				setStopwatch4(500);
				startStopwatch4();
				avoid.state = AVOID_END;
			}
			last_obstacle = RIGHT;
			obstacle_counter++;
		break;
		case AVOID_OBSTACLE_LEFT:
			avoid.dir = FWD;
			avoid.speed_left = AVOID_SPEED_R_ARC_LEFT;
			avoid.speed_right = AVOID_SPEED_R_ARC_RIGHT;
			if(obstacle_right && obstacle_left)
				avoid.state = AVOID_OBSTACLE_MIDDLE;
			if(!obstacle_left)
			{
				setStopwatch4(500); 
				startStopwatch4();
				avoid.state = AVOID_END;
			}
			last_obstacle = LEFT;
			obstacle_counter++;
		break;
		case AVOID_END:
			if(getStopwatch4() > 1000) //
			{
				stopStopwatch4();
				setStopwatch4(0);
				avoid.state = IDLE;
			}
		break;
	}
}

/**
 LED de controlle des capteur IR
 */
void acsStateChanged(void)
{
	if(obstacle_left && obstacle_right)
		statusLEDs.byte = 0b100100;
	else
		statusLEDs.byte = 0b000000;
	statusLEDs.LED5 = obstacle_left;
	statusLEDs.LED4 = (!obstacle_left);
	statusLEDs.LED2 = obstacle_right;
	statusLEDs.LED1 = (!obstacle_right);
	updateStatusLEDs();
}

/*****************************************************************************/
//  control et generation du mouvement:


void moveCommand(behaviour_command_t * cmd)
{
	if(cmd->move_value > 0)  // movement ou rotation?
	{
		if(cmd->rotate)
			rotate(cmd->speed_left, cmd->dir, cmd->move_value, false); 
		else if(cmd->move)
			move(cmd->speed_left, cmd->dir, DIST_MM(cmd->move_value), false); 
		cmd->move_value = 0; 
	}
	else if(!(cmd->move || cmd->rotate)) 
	{
		changeDirection(cmd->dir);
		moveAtSpeed(cmd->speed_left,cmd->speed_right);
	}
	else if(isMovementComplete()) // movement complete? --> change flags!
	{
		cmd->rotate = false;
		cmd->move = false;
	}
}


void behaviourController(void)
{
    // fonction behavior:
	behaviour_cruise();
	behaviour_avoid();
	behaviour_escape();

    // Execute les commande avec le valeur de priorité:
	if(escape.state != IDLE) //  priorité - 3
		moveCommand(&escape);
	else if(avoid.state != IDLE) // Priorité - 2
		moveCommand(&avoid);
	else if(cruise.state != IDLE) // Priorité - 1
		moveCommand(&cruise); 
	else                     //  priorité - 0
		moveCommand(&STOP);  
}

/*****************************************************************************/
// Main:

int main(void)
{
	initRobotBase(); 
	setLEDs(0b111111);
	mSleep(2500);
	setLEDs(0b100100); 

	// gestionnaire d'evenement bumper
	BUMPERS_setStateChangedHandler(bumpersStateChanged);
	
	// gestionnaire d'evenement infrarouge
	ACS_setStateChangedHandler(acsStateChanged);
	
	powerON(); 
	setACSPwrMed(); 

	while(true) 
	{		
		behaviourController();
		task_RP6System();
	}
	return 0;
}
