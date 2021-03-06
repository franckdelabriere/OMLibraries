// 
// 
// 

#include "key_frames.h"
#include "hermite_spline.h"

namespace globalKF{
	int something = 3;
}

// Default constructor
KeyFrames::KeyFrames(){
	m_xn = NULL;
	m_fn = NULL;
	m_dn = NULL;
	m_xn_recieved = 0;
	m_fn_recieved = 0;
	m_dn_recieved = 0;
	m_kf_count = 0;
}

// Default destructor
KeyFrames::~KeyFrames(){
	freeMemory();
}

// Initialize static class variables
const int	KeyFrames::G_VALIDATION_PNT_COUNT = 1000;
int			KeyFrames::g_cur_axis = 0;
bool		KeyFrames::g_receiving = false;
int			KeyFrames::g_update_rate = 10;
bool		KeyFrames::g_mem_allocted = false;
KeyFrames*	KeyFrames::g_axis_array = NULL;
int			KeyFrames::g_axis_count = 0;
float		KeyFrames::g_max_accel = 20000;
float		KeyFrames::g_max_vel = 4000;
long		KeyFrames::g_cont_vid_time = -1;

/*** Static Functions ***/

// Selects the the current axis
void KeyFrames::setContVidTime(long p_time){
	g_cont_vid_time = p_time;
}

// Returns the currently selected axis	
long KeyFrames::getContVidTime(){
	return g_cont_vid_time;
}

// Points the axis_array var to an existing array of key frames objects that represent the axes to be managed
void KeyFrames::setAxisArray(KeyFrames* p_axis_array, int p_axis_count){
	g_axis_array = p_axis_array;
	g_axis_count = p_axis_count;
}

// Selects the the current axis
void KeyFrames::setAxis(int p_axis){
	g_cur_axis = p_axis;
}

// Returns the currently selected axis	
int KeyFrames::getAxis(){
	return g_cur_axis;
}

// Returns the number of axes being managed
int KeyFrames::getAxisCount(){
	return	g_axis_count;
}

// Sets the velocity update rate in ms used at run-time
void KeyFrames::updateRate(int p_updates_per_sec){
	g_update_rate = p_updates_per_sec;
}

// Returns the velocity update rate in ms
int KeyFrames::updateRate(){
	return g_update_rate;
}

// Set whether the NMX is currently receiving key frame input data
void KeyFrames::receiveState(bool p_state){
	g_receiving = p_state;
}

// Returns whether the NMX is currently receiving key frame input data
bool KeyFrames::receiveState(){
	return g_receiving;
}

// Sets the key frame count and allocates memory for input vars
void KeyFrames::setKFCount(int p_kf_count){

	// Don't allow negative counts or a single frame
	if (p_kf_count < 0)
		return;

	m_kf_count = p_kf_count;

	// Only allocate memory for 2 or more frames. Frame counts of 0 or 1 are just used as indicators
	if (p_kf_count >= 2){

		// Free any memory that may have been already allocated
		freeMemory();		

		// Allocate memory for and reset the received values for each		
		m_xn = (float *)malloc(m_kf_count * sizeof(float));
		m_fn = (float *)malloc(m_kf_count * sizeof(float));
		m_dn = (float *)malloc(m_kf_count * sizeof(float));
		m_xn_recieved = 0;
		m_fn_recieved = 0;
		m_dn_recieved = 0;			
		
		g_mem_allocted = true;
	}
}

// Returns the key frame count
int KeyFrames::getKFCount(){
	return m_kf_count;
}

// Points xn to an existing array of values
void KeyFrames::setXN(float* p_xn){

	if (!g_mem_allocted)
		return;

	m_xn = p_xn;
}

// Assigns xn values one at a time	
void KeyFrames::setXN(float p_input){
	m_xn[m_xn_recieved] = p_input;
	m_xn_recieved++;
}

// Returns the number of xn values that have been assigned. Accurate only when assigning values one at a time.
int KeyFrames::countXN(){
	return m_xn_recieved;
}

// Resets the xn received count
void KeyFrames::resetXN(){
	m_xn_recieved = 0;
}

// Returns the abscissa of the requested key frame
float KeyFrames::getXN(int p_which){
	return m_xn[p_which];
}

// Returns the largest of the final xn values for all axes. This is useful for determining the length of a program.
float KeyFrames::getMaxLastXN(){

	float max_xn = 0;

	for (byte i = 0; i < KeyFrames::g_axis_count; i++){		
		int max_frame_num = KeyFrames::g_axis_array[i].m_kf_count;		
		float this_xn = KeyFrames::g_axis_array[i].m_xn[max_frame_num-1];		
		if (this_xn > max_xn)
			max_xn = this_xn;
	}
		
	return max_xn;
}

// Deallocates any memory assigned to input arrays
void KeyFrames::freeMemory(){
	// If the count has been previously set, deallocate the memory from last time
	if (g_mem_allocted){
		free(m_xn);		
		free(m_fn);
		free(m_dn);
		
		g_mem_allocted = false;
	}
}

void KeyFrames::setFN(float* p_fn){
	if (!g_mem_allocted)
		return;
	m_fn = p_fn;
}

void KeyFrames::setFN(float p_input){
	if (!g_mem_allocted)
		return;
	m_fn[m_fn_recieved] = p_input;	
	m_fn_recieved++;
}

int KeyFrames::countFN(){
	return m_fn_recieved;
}

// Resets the fn received count
void KeyFrames::resetFN(){
	m_fn_recieved= 0;
}

float KeyFrames::getFN(int p_which){
	return m_fn[p_which];
}

void KeyFrames::setDN(float* p_dn){
	if (!g_mem_allocted)
		return;
	m_dn = p_dn;
}

void KeyFrames::setDN(float p_input){
	if (!g_mem_allocted)
		return; 
	m_dn[m_dn_recieved] = p_input;
	m_dn_recieved++;
}

int KeyFrames::countDN(){
	return m_dn_recieved;
}

// Resets the fn received count
void KeyFrames::resetDN(){
	m_dn_recieved = 0;
}

float KeyFrames::getDN(int p_which){
	return m_dn[p_which];
}

float KeyFrames::pos(float p_x){
	updateVals(p_x);
	return m_f[0];
}

float KeyFrames::vel(float p_x){
	updateVals(p_x);
	return m_d[0];
}

float KeyFrames::accel(float p_x){
	updateVals(p_x);
	return m_s[0];
}

/*** Validation Functions ***/

bool KeyFrames::validateVel(){

	float increment = m_xn[m_kf_count - 1] / G_VALIDATION_PNT_COUNT - 1;	
	
	for (int i = 0; i < G_VALIDATION_PNT_COUNT; i++){
		updateVals(i * increment);
		if (abs(m_d[0]) > g_max_vel)
			return false;
	}
	return true;
}

bool KeyFrames::validateAccel(){

	float increment = m_xn[m_kf_count - 1] / G_VALIDATION_PNT_COUNT - 1;
	
	for (int i = 0; i < G_VALIDATION_PNT_COUNT; i++){
		updateVals(i * increment);
		if (abs(m_s[0]) > g_max_accel)
			return false;
	}
	return true;
}

void KeyFrames::setMaxVel(float p_max_vel){
	g_max_vel = p_max_vel;
}

void KeyFrames::setMaxAccel(float p_max_accel){
	g_max_accel = p_max_accel;
}

/*** Non-Static Private Functions ***/

void KeyFrames::updateVals(float p_x){
	float x_point[1];
	
	// Don't allow requests for x values less than the first point and greater than the last point
	if (p_x < m_xn[0])
		x_point[0] = m_xn[0];
	else if (p_x > m_xn[m_xn_recieved - 1])
		x_point[0] = m_xn[m_xn_recieved - 1];
	else
		x_point[0] = p_x;

	HermiteSpline::cubic_spline_value(m_kf_count, m_xn, m_fn, m_dn, 1, x_point, m_f, m_d, m_s);
}