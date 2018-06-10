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

const int GESTO_VACIO = 0;
const int GESTO_LIMPIAR_DATOS = 1;

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
					// Gesto: Limpiar Datos
					bool gesto_limpiar_datos = frame.hands().count();
					// Obteniendo datos del Leap Motion
					for (const Leap::Hand& hand : frame.hands())
					{
						_V_leap_motion->_V_tracked[hand.isRight()] = true;
						int finger_id = 0;
						for (const Leap::Finger& finger : hand.fingers())
						{
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
							_V_leap_motion->_V_pos[hand.isRight()][finger_id] = bone_meta.prevJoint();
							++finger_id;
							// Gesto: Limpiar Datos
							if (gesto_limpiar_datos)
							{
								const Leap::Bone bone_prox = finger.bone(Leap::Bone::Type::TYPE_PROXIMAL);
								const Leap::Bone bone_inter = finger.bone(Leap::Bone::Type::TYPE_INTERMEDIATE);
								const Leap::Bone bone_distal = finger.bone(Leap::Bone::Type::TYPE_DISTAL);
								Leap::Vector finger_direction = (bone_prox.direction() + bone_inter.direction() + bone_distal.direction()) / 3.0;
								finger_direction.y = 0; // Eliminamos el eje_y para no considerar la inclinación de la mano
								// Validación del Dedo
								float angulo_formado = LL::radian_to_sexagesimal(finger_direction.angleTo(Leap::Vector(0, 0, 1)));
								switch (finger.type())
								{
									case Leap::Finger::TYPE_THUMB:
									{
										gesto_limpiar_datos &= (angulo_formado > _V_leap_motion->_V_thumb_angle &&
																angulo_formado < _V_leap_motion->_V_finger_angle + _V_leap_motion->_V_thumb_angle);
										break;
									}
									case Leap::Finger::TYPE_INDEX:
									case Leap::Finger::TYPE_MIDDLE:
									{
										gesto_limpiar_datos &= (angulo_formado > 0 &&
																angulo_formado < _V_leap_motion->_V_finger_angle);
										break;
									}
									default:
									{
										break;
									}
								}
							}
						}
					}
					if (gesto_limpiar_datos && _V_leap_motion->_V_tracked[0] && _V_leap_motion->_V_tracked[1])
						_V_leap_motion->_V_gesto_actual = GESTO_LIMPIAR_DATOS;
				}
				virtual void onDisconnect(const Leap::Controller&)
				{
					_V_leap_motion->_V_leap_connected = false;
				}
		};;
		bool _V_leap_connected = false;
		LL_AL5::Button* _V_leap_motion_button = nullptr;
		// Datos
		bool _V_reading = false;
		int _V_gesto_actual = GESTO_VACIO;
		bool _V_tracked[NUMBER_HAND];
		Leap::Vector _V_dir[NUMBER_HAND][NUMBER_FINGER][NUMBER_BONES];
		Leap::Vector _V_pos[NUMBER_HAND][NUMBER_FINGER];
		// Gestos
		// Limpiar Datos
		int _V_thumb_angle=25;
		int _V_finger_angle=30;
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
	public:
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
						data.push_back(_V_pos[hand][finger].x);
						data.push_back(_V_pos[hand][finger].y);
						data.push_back(_V_pos[hand][finger].z);
						for (int bone = 0; bone < NUMBER_BONES; ++bone)
						{
							data.push_back(_V_dir[hand][finger][bone].x);
							data.push_back(_V_dir[hand][finger][bone].y);
							data.push_back(_V_dir[hand][finger][bone].z);
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
		}
};

#endif // INCLUDED_LEAP_MOTION_H
