#ifndef INCLUDED_LEAP_MOTION_H
#define INCLUDED_LEAP_MOTION_H

#include "LexRisLogic\include\LexRisLogic\Allegro5\Allegro5.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Color.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Display.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Input.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Text.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Primitives.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Special\Interface.h"

#include "LexRisLogic\include\LexRisLogic\Math.h"

#include <Leap.h>

#include <list>

const int NUMBER_HAND = 2;
const int NUMBER_FINGER = 5;
const int NUMBER_BONES = 4;

const int LEFT_HAND = 0;
const int RIGHT_HAND = 1;

const int TOTAL_AXIS = 3;

const int GESTO_VACIO = 0;
const int GESTO_TOMAR_FOTO = 1;
const int GESTO_INTERACTUAR_IZQ = 2;
const int GESTO_INTERACTUAR_DER = 3;
const int GESTO_DEJAR_IZQ = 4;
const int GESTO_DEJAR_DER = 5;
const int GESTO_OPCION_COMPARTIR_IZQ = 6;
const int GESTO_OPCION_COMPARTIR_DER = 7;
const int GESTO_OPCION_DEJAR_COMPARTIR_IZQ = 8;
const int GESTO_OPCION_DEJAR_COMPARTIR_DER = 9;
const int GESTO_OPCION_ELIMINAR_IZQ = 10;
const int GESTO_OPCION_ELIMINAR_DER = 11;

class GestureDetection
{
	private:
		struct _S_structure_FingerStatus
		{
			bool test = true;
			Leap::Vector base_vector;
			float limit_angle = 0.0;
		};
		_S_structure_FingerStatus _V_base[NUMBER_HAND][NUMBER_FINGER];
		bool _V_remove_axis[TOTAL_AXIS];
		bool _V_hand_enabled[NUMBER_HAND];
	public:
		GestureDetection(Leap::Vector base_vector, float limit_angle)
		{
			for(int i = 0; i < TOTAL_AXIS; ++i)
			{
				_V_remove_axis[i] = false;
			}
			set_all_base_vector(base_vector, limit_angle);
			_V_hand_enabled[0] = true;
			_V_hand_enabled[1] = true;
		}
		void set_all_base_vector(Leap::Vector base_vector, float limit_angle)
		{
			for (int hand = 0; hand < NUMBER_HAND; ++hand)
			{
				for (int finger = 0; finger < NUMBER_FINGER; ++finger)
				{
					_V_base[hand][finger].base_vector = base_vector;
					_V_base[hand][finger].limit_angle = limit_angle;
				}
			}
		}
		_S_structure_FingerStatus& operator () (int hand, int finger)
		{
			return _V_base[hand][finger];
		}
		bool& operator [] (int axis)
		{
			return _V_remove_axis[axis];
		}
		void enable_hand(int hand, bool value)
		{
			_V_hand_enabled[hand] = value;
		}
		bool collision(Leap::Vector finger_direction[NUMBER_HAND][NUMBER_FINGER])
		{
			for (int hand = 0; hand < NUMBER_HAND; ++hand)
			{
				if (_V_hand_enabled[hand]) {
					for (int finger = 0; finger < NUMBER_FINGER; ++finger)
					{
						_S_structure_FingerStatus& data = _V_base[hand][finger];
						if (data.test)
						{
							Leap::Vector dir = finger_direction[hand][finger];
							dir.x *= (!_V_remove_axis[0]);
							dir.y *= (!_V_remove_axis[1]);
							dir.z *= (!_V_remove_axis[2]);
							float angulo_formado = LL::radian_to_sexagesimal(dir.angleTo(data.base_vector));
							if (angulo_formado > data.limit_angle) {
								//std::cout << "Error: H: " << hand << " F:" << finger << std::endl;
								//std::cout << "Error: A: " << angulo_formado << " L:" << data.limit_angle << std::endl;
								return false;
							}
						}
					}
				}
			}
			return true;
		}
};

class LeapMotion
{
	private:
		class _C_Class_Listener : public Leap::Listener
		{
			private:
				LeapMotion * _V_leap_motion = nullptr;
			public:
				_C_Class_Listener(LeapMotion* parent)
				{
					_V_leap_motion = parent;
				}
				virtual void onConnect(const Leap::Controller&)
				{
					_V_leap_motion->_V_leap_connected = true;
				}	
				virtual void onFrame(const Leap::Controller& controller)
				{
					if (_V_leap_motion->_V_reading)
						return;
					_V_leap_motion->_F_clear();
					const Leap::Frame frame = controller.frame();
					// Obteniendo datos del Leap Motion
					Leap::Vector finger_direction[NUMBER_HAND][NUMBER_FINGER];
					for (const Leap::Hand& hand : frame.hands())
					{
						_V_leap_motion->_V_tracked[hand.isRight()] = true;
						for (const Leap::Finger& finger : hand.fingers())
						{
							int finger_id = finger.type();
							for (int bone_id = 0; bone_id < NUMBER_BONES; ++bone_id)
							{
								Leap::Bone::Type type;
								switch (bone_id) {
								case 0:
									type = Leap::Bone::Type::TYPE_METACARPAL;
									break;
								case 1:
									type = Leap::Bone::Type::TYPE_PROXIMAL;
									break;
								case 2:
									type = Leap::Bone::Type::TYPE_INTERMEDIATE;
									break;
								case 3:
									type = Leap::Bone::Type::TYPE_DISTAL;
									break;
								}
								const Leap::Bone bone = finger.bone(type);
								_V_leap_motion->_V_dir[hand.isRight()][finger_id][bone_id] = bone.nextJoint() - bone.prevJoint();
							}
							const Leap::Bone bone_meta = finger.bone(Leap::Bone::Type::TYPE_METACARPAL);
							const Leap::Bone bone_prox = finger.bone(Leap::Bone::Type::TYPE_PROXIMAL);
							const Leap::Bone bone_inter = finger.bone(Leap::Bone::Type::TYPE_INTERMEDIATE);
							const Leap::Bone bone_distal = finger.bone(Leap::Bone::Type::TYPE_DISTAL);
							_V_leap_motion->_V_pos[hand.isRight()][finger_id] = bone_meta.prevJoint();
							finger_direction[hand.isRight()][finger_id] = (bone_prox.direction() + bone_inter.direction() + bone_distal.direction()) / 3.0;
						}
					}
					if (_V_leap_motion->_V_tracked[0] && _V_leap_motion->_V_tracked[1])
					{
						if(_V_leap_motion->_V_gesture_take_photo_1->collision(finger_direction) || _V_leap_motion->_V_gesture_take_photo_2->collision(finger_direction))
							_V_leap_motion->_V_gesto_actual = GESTO_TOMAR_FOTO;
						// Mano Izquierda Apuntando
						if (_V_leap_motion->_V_gesture_interact_left->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_INTERACTUAR_IZQ;
							if (_V_leap_motion->_V_gesture_opcion_4_right->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_COMPARTIR_DER;
							else if (_V_leap_motion->_V_gesture_opcion_5_right->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_DEJAR_COMPARTIR_DER;
							else if (_V_leap_motion->_V_gesture_opcion_6_right->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_ELIMINAR_DER;
						}
						// Mano Derecha Apuntando
						else if (_V_leap_motion->_V_gesture_interact_right->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_INTERACTUAR_DER;
							if (_V_leap_motion->_V_gesture_opcion_4_left->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_COMPARTIR_IZQ;
							else if (_V_leap_motion->_V_gesture_opcion_5_left->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_DEJAR_COMPARTIR_IZQ;
							else if (_V_leap_motion->_V_gesture_opcion_6_left->collision(finger_direction))
								_V_leap_motion->_V_gesto_actual = GESTO_OPCION_ELIMINAR_IZQ;
						}
						else if (_V_leap_motion->_V_gesture_stop_left->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_DEJAR_IZQ;
						}
						else if (_V_leap_motion->_V_gesture_stop_right->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_DEJAR_DER;
						}
					}
					else if (_V_leap_motion->_V_tracked[0])
					{
						if (_V_leap_motion->_V_gesture_interact_left->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_INTERACTUAR_IZQ;
						}
						else if (_V_leap_motion->_V_gesture_stop_left->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_DEJAR_IZQ;
						}
					}
					else if (_V_leap_motion->_V_tracked[1])
					{
						if (_V_leap_motion->_V_gesture_interact_right->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_INTERACTUAR_DER;
						}
						else if (_V_leap_motion->_V_gesture_stop_right->collision(finger_direction)) {
							_V_leap_motion->_V_gesto_actual = GESTO_DEJAR_DER;
						}
					}
				}
				virtual void onDisconnect(const Leap::Controller&)
				{
					_V_leap_motion->_V_leap_connected = false;
				}
		};
		bool _V_leap_connected = false;
		LL_AL5::Button* _V_leap_motion_button = nullptr;
		// Datos
		bool _V_reading = false;
		int _V_gesto_actual = GESTO_VACIO;
		bool _V_tracked[NUMBER_HAND];
		Leap::Vector _V_dir[NUMBER_HAND][NUMBER_FINGER][NUMBER_BONES];
		Leap::Vector _V_pos[NUMBER_HAND][NUMBER_FINGER];
		// Gestos
		GestureDetection* _V_gesture_take_photo_1 = nullptr;
		GestureDetection* _V_gesture_take_photo_2 = nullptr;
		GestureDetection* _V_gesture_interact_left = nullptr;
		GestureDetection* _V_gesture_interact_right = nullptr;
		GestureDetection* _V_gesture_stop_left = nullptr;
		GestureDetection* _V_gesture_stop_right = nullptr;
		GestureDetection* _V_gesture_opcion_4_left = nullptr;
		GestureDetection* _V_gesture_opcion_5_left = nullptr;
		GestureDetection* _V_gesture_opcion_6_left = nullptr;
		GestureDetection* _V_gesture_opcion_4_right = nullptr;
		GestureDetection* _V_gesture_opcion_5_right = nullptr;
		GestureDetection* _V_gesture_opcion_6_right = nullptr;
		// Funciones
		void _F_disable()
		{
			if(_V_leap_motion_button)
			{
				_V_leap_motion_button->set_unclick_fill_color(LL_AL5::Color(255));
				_V_leap_motion_button->set_click_fill_color(LL_AL5::Color(255));
				_V_leap_motion_button->set_button_text("Leap Desconectado    ");
			}
		}
		void _F_enable()
		{
			if (_V_leap_motion_button)
			{
				_V_leap_motion_button->set_unclick_fill_color(LL_AL5::Color(0, 255));
				_V_leap_motion_button->set_click_fill_color(LL_AL5::Color(0, 255));
				_V_leap_motion_button->set_button_text("Leap Conectado       ");
			}
		}
		void _F_clear()
		{
			_V_gesto_actual = GESTO_VACIO;
			for (int hand = 0; hand<NUMBER_HAND; ++hand)
			{
				_V_tracked[hand] = false;
				for (int finger = 0; finger<NUMBER_FINGER; ++finger)
				{
					_V_pos[hand][finger].x = 0;
					_V_pos[hand][finger].y = 0;
					_V_pos[hand][finger].z = -9999;
					for (int bone = 0; bone < NUMBER_BONES; ++bone)
					{
						_V_dir[hand][finger][bone].x = 0;
						_V_dir[hand][finger][bone].y = 0;
						_V_dir[hand][finger][bone].z = 0;
					}
				}
			}
		}
		_C_Class_Listener* _V_listener=nullptr;
		Leap::Controller _V_controller;
		LL::Chronometer _V_timer;
	public:
		LeapMotion()
		{
			// Gestos
			// Tomar Fotos
			_V_gesture_take_photo_1 = new GestureDetection(Leap::Vector(0, 0, 0), 30);
			{
				(*_V_gesture_take_photo_1)(LEFT_HAND, Leap::Finger::TYPE_THUMB).base_vector = Leap::Vector(0.5, 0, -1);
				(*_V_gesture_take_photo_1)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).base_vector = Leap::Vector(-0.5, 0, 1);
				(*_V_gesture_take_photo_1)(LEFT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(1, 0, 0);
				(*_V_gesture_take_photo_1)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(-1, 0, 0);
				(*_V_gesture_take_photo_1)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_take_photo_1)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_take_photo_1)(LEFT_HAND, Leap::Finger::TYPE_RING).test = false;
				(*_V_gesture_take_photo_1)(RIGHT_HAND, Leap::Finger::TYPE_RING).test = false;
				(*_V_gesture_take_photo_1)(LEFT_HAND, Leap::Finger::TYPE_MIDDLE).test = false;
				(*_V_gesture_take_photo_1)(RIGHT_HAND, Leap::Finger::TYPE_MIDDLE).test = false;
				(*_V_gesture_take_photo_1)[1] = true;
			}
			_V_gesture_take_photo_2 = new GestureDetection(Leap::Vector(0, 0, 0), 30);
			{
				(*_V_gesture_take_photo_2)(LEFT_HAND, Leap::Finger::TYPE_THUMB).base_vector = Leap::Vector(0.5, 0, 1);
				(*_V_gesture_take_photo_2)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).base_vector = Leap::Vector(-0.5, 0, -1);
				(*_V_gesture_take_photo_2)(LEFT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(1, 0, 0);
				(*_V_gesture_take_photo_2)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(-1, 0, 0);
				(*_V_gesture_take_photo_2)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_take_photo_2)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_take_photo_2)(LEFT_HAND, Leap::Finger::TYPE_RING).test = false;
				(*_V_gesture_take_photo_2)(RIGHT_HAND, Leap::Finger::TYPE_RING).test = false;
				(*_V_gesture_take_photo_2)(LEFT_HAND, Leap::Finger::TYPE_MIDDLE).test = false;
				(*_V_gesture_take_photo_2)(RIGHT_HAND, Leap::Finger::TYPE_MIDDLE).test = false;
				(*_V_gesture_take_photo_2)[1] = true;
			}
			// Interactuar
			_V_gesture_interact_left = new GestureDetection(Leap::Vector(0, 1, -0.75), 77);
			{
				(*_V_gesture_interact_left).enable_hand(1, false);
				(*_V_gesture_interact_left)(LEFT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, -1, 0);
				(*_V_gesture_interact_left)(LEFT_HAND, Leap::Finger::TYPE_INDEX).limit_angle = 90;
				(*_V_gesture_interact_left)(LEFT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_interact_left)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_interact_left)[2] = true;
			}
			_V_gesture_interact_right = new GestureDetection(Leap::Vector(0, 1, -0.75), 77);
			{
				(*_V_gesture_interact_right).enable_hand(0, false);
				(*_V_gesture_interact_right)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, -1, 0);
				(*_V_gesture_interact_right)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).limit_angle = 90;
				(*_V_gesture_interact_right)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_interact_right)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_interact_right)[2] = true;
			}
			//Dejar
			_V_gesture_stop_left = new GestureDetection(Leap::Vector(0, 1, -0.75), 77);
			{
				(*_V_gesture_stop_left).enable_hand(1, false);
				(*_V_gesture_stop_left)(LEFT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_stop_left)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
			}
			_V_gesture_stop_right = new GestureDetection(Leap::Vector(0, 1, -0.75), 77);
			{
				(*_V_gesture_stop_right).enable_hand(0, false);
				(*_V_gesture_stop_right)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_stop_right)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
			}
			//Opciones
			//Compartir
			_V_gesture_opcion_4_left = new GestureDetection(Leap::Vector(0, 0, -1), 35);
			{
				(*_V_gesture_opcion_4_left).enable_hand(1, false);
				(*_V_gesture_opcion_4_left)(LEFT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_4_left)(LEFT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_4_left)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_4_left)[1] = true;
			}
			_V_gesture_opcion_4_right = new GestureDetection(Leap::Vector(0, 0, -1), 35);
			{
				(*_V_gesture_opcion_4_right).enable_hand(0, false);
				(*_V_gesture_opcion_4_right)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_4_right)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_4_right)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_4_right)[1] = true;
			}
			//Dejar de Compartir
			_V_gesture_opcion_5_left = new GestureDetection(Leap::Vector(0, 0, -1), 35);
			{
				(*_V_gesture_opcion_5_left).enable_hand(1, false);
				(*_V_gesture_opcion_5_left)(LEFT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_5_left)(LEFT_HAND, Leap::Finger::TYPE_MIDDLE).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_5_left)(LEFT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_5_left)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_5_left)[1] = true;
			}
			_V_gesture_opcion_5_right = new GestureDetection(Leap::Vector(0, 0, -1), 40);
			{
				(*_V_gesture_opcion_5_right).enable_hand(0, false);
				(*_V_gesture_opcion_5_right)(RIGHT_HAND, Leap::Finger::TYPE_INDEX).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_5_right)(RIGHT_HAND, Leap::Finger::TYPE_MIDDLE).base_vector = Leap::Vector(0, 0, 1);
				(*_V_gesture_opcion_5_right)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_5_right)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_5_right)[1] = true;
			}
			//Eliminar
			_V_gesture_opcion_6_left = new GestureDetection(Leap::Vector(0, 0, 1), 35);
			{
				(*_V_gesture_opcion_6_left).enable_hand(1, false);
				(*_V_gesture_opcion_6_left)(LEFT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_6_left)(LEFT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_6_left)[1] = true;
			}
			_V_gesture_opcion_6_right = new GestureDetection(Leap::Vector(0, 0, 1), 35);
			{
				(*_V_gesture_opcion_6_right).enable_hand(0, false);
				(*_V_gesture_opcion_6_right)(RIGHT_HAND, Leap::Finger::TYPE_THUMB).test = false;
				(*_V_gesture_opcion_6_right)(RIGHT_HAND, Leap::Finger::TYPE_PINKY).test = false;
				(*_V_gesture_opcion_6_right)[1] = true;
			}
			/**/
			_V_controller.config().setBool("tracking_processing_auto_flip", true);
			_V_controller.setPolicy(Leap::Controller::PolicyFlag::POLICY_OPTIMIZE_HMD);
			_V_timer.stop();
		}
		void create(LL_AL5::Input* input, LL_AL5::Font* font,float pos_x,float pos_y)
		{
			_F_clear();
			_V_leap_motion_button = new LL_AL5::Button(input);
			_V_leap_motion_button->set_font(font);
			_V_leap_motion_button->set_pos(pos_x, pos_y);
			_V_leap_motion_button->button_off();
			_V_listener = new _C_Class_Listener(this);
			if(_V_listener)
				_V_controller.addListener(*_V_listener);
		}
		void draw(LL_AL5::Display* display)
		{
			if (_V_leap_connected)
				_F_enable();
			else
				_F_disable();
			display->draw(_V_leap_motion_button);
		}
		int get_gesture()
		{
			return _V_gesto_actual;
		}
		void get_data(std::list<float>& data)
		{
			/*Protocolo*/
			/*
				[4] - ID del Gesto
				[4] - Mano Izquierda Estado
				[4] - Mano Derecha Estado
				[(5) Dedos por Mano
					[12] - Posición XYZ por Dedo
					[(4) Huesos Por Dedo
						[12] - Vector XYZ por Hueso
					] = 48
				] = 300
				Manos sin ser Rastreadas:       12 bytes
				Mano Izquierda Rastreada:       312 bytes
				Mano Derecha Rastreada:         312 bytes
				Manos Rastreadas:               612 bytes
			*/
			_V_reading = true;
			data.clear();
			data.push_back(_V_gesto_actual);
			data.push_back(_V_tracked[0]);
			data.push_back(_V_tracked[1]);
			for (int hand = 0; hand<NUMBER_HAND; ++hand)
			{
				if (_V_tracked[hand])
				{
					for (int finger = 0; finger < NUMBER_FINGER; ++finger)
					{
						data.push_back(_V_pos[hand][finger].x * -1);
						data.push_back(_V_pos[hand][finger].z * -1);
						data.push_back(_V_pos[hand][finger].y + 100);
						for (int bone = 0; bone < NUMBER_BONES; ++bone)
						{
							data.push_back(_V_dir[hand][finger][bone].x * -1);
							data.push_back(_V_dir[hand][finger][bone].z * -1);
							data.push_back(_V_dir[hand][finger][bone].y);
						}
					}
				}
			}
			_V_reading = false;
		}
		~LeapMotion()
		{
			if (_V_listener)
			{
				_V_controller.removeListener(*_V_listener);
				delete(_V_listener);
			}
			_V_listener = nullptr;
			if(_V_leap_motion_button)
				delete(_V_leap_motion_button);
			_V_leap_motion_button = nullptr;
			// Eliminar Gestos
			delete(_V_gesture_take_photo_1);
			delete(_V_gesture_take_photo_2);
			delete(_V_gesture_interact_left);
			delete(_V_gesture_interact_right);
			delete(_V_gesture_stop_left);
			delete(_V_gesture_stop_right);
			delete(_V_gesture_opcion_4_left);
			delete(_V_gesture_opcion_5_left);
			delete(_V_gesture_opcion_6_left);
			delete(_V_gesture_opcion_4_right);
			delete(_V_gesture_opcion_5_right);
			delete(_V_gesture_opcion_6_right);
		}
};

#endif // INCLUDED_LEAP_MOTION_H
