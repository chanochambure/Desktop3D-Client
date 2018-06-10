#include <iostream>

#include "LexRisLogic\include\LexRisLogic\Allegro5\Allegro5.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Color.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Display.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Input.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Text.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Primitives.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Special\Interface.h"
#include "LexRisLogic\include\LexRisLogic\Convert.h"
#include "LexRisLogic\include\LexRisLogic\Time.h"

#include "LeapMotion.h"
#include "Network.h"

int main()
{
	// Inicialización Allegro
	LL_AL5::init_allegro();
	LL_AL5::primitives_addon();
	LL_AL5::text_addon();
	// Temporizador
	float time_get_connection = 0.5;
	float time_send_package = 0.09;
	LL_AL5::Display display(800, 450);
	{
		display.set_title("Desktop 3D");
		display.create();
	}
	LL_AL5::KeyControl key_control;
	{
	}
	LL_AL5::Input input;
	{
		input.register_display(display);
		input.keyboard_on();
		input.mouse_on(true);
		input.set_key_control(&key_control);
	}
	// Recursos
	LL_AL5::Font agency_font;
	{
		agency_font.set_path("agency.ttf");
		agency_font.set_size(15);
		agency_font.load_ttf_font();
	}
	LL::Chronometer timer_get_connection;
	{
		timer_get_connection.clear();
		timer_get_connection.play();
	}
	LL::Chronometer timer_send_package;
	{
		timer_send_package.clear();
		timer_send_package.play();
	}
	// Interfaces de Conexión
	LeapMotion leap_motion;
	{
		leap_motion.create(&input, &agency_font, 550, 450 - agency_font.get_size()*1.25);
	}
	Network network;
	{
		network.create_server(&input, &agency_font, 0, 430 - agency_font.get_size()*1.25);
		network.create_conn(&input, &agency_font, 0, 450 - agency_font.get_size()*1.25);
	}
	// Interfaces de Aplicación
	LL_AL5::TextBox textbox_ip(&input);
	{
		textbox_ip.set_font(&agency_font);
		textbox_ip.set_click_line_color(LL_AL5::Color(0, 255));
		textbox_ip.set_thickness(2);
		textbox_ip.set_text_length(15);
		textbox_ip.textbox_on();
		textbox_ip.set_pos(150, 340);
		textbox_ip.set_value("127.0.0.1");
	}
	LL_AL5::TextBox textbox_port(&input);
	{
		textbox_port.set_font(&agency_font);
		textbox_port.set_click_line_color(LL_AL5::Color(0, 255));
		textbox_port.set_thickness(2);
		textbox_port.set_text_length(5);
		textbox_port.textbox_on();
		textbox_port.set_pos(150, 360);
		textbox_port.set_value("8888");
	}
	LL_AL5::TextBox textbox_conn(&input);
	{
		textbox_conn.set_font(&agency_font);
		textbox_conn.set_click_line_color(LL_AL5::Color(0, 255));
		textbox_conn.set_thickness(2);
		textbox_conn.set_text_length(10);
		textbox_conn.textbox_on();
		textbox_conn.set_pos(620, 340);
		textbox_conn.set_value(LL::to_string(time_get_connection));
	}
	LL_AL5::TextBox textbox_send(&input);
	{
		textbox_send.set_font(&agency_font);
		textbox_send.set_click_line_color(LL_AL5::Color(0, 255));
		textbox_send.set_thickness(2);
		textbox_send.set_text_length(10);
		textbox_send.textbox_on();
		textbox_send.set_pos(620, 360);
		textbox_send.set_value(LL::to_string(time_send_package));
	}
	LL_AL5::Button change_button(&input);
	{
		change_button.set_click_fill_color(LL_AL5::Color(0, 128, 0));
		change_button.set_pos(620, 380);
		change_button.set_font(&agency_font);
		change_button.set_button_text("Cambiar Tiempos");
		change_button.button_on();
	}
	LL_AL5::Button server_button(&input);
	{
		server_button.set_click_fill_color(LL_AL5::Color(0, 128, 0));
		server_button.set_pos(150, 380);
		server_button.set_font(&agency_font);
		server_button.set_button_text("Iniciar");
		server_button.button_on();
	}
	// Texto
	LL_AL5::Text text_error_message;
	{
		text_error_message.set_font(&agency_font);
		text_error_message.set_pos(400, 20);
		text_error_message.set_flag(ALLEGRO_ALIGN_CENTER);
		text_error_message.set_color(LL_AL5::Color(255,128));
		text_error_message = "";
	}
	LL_AL5::Text text_ip;
	{
		text_ip.set_font(&agency_font);
		text_ip.set_pos(10, 340);
		text_ip.set_color(LL_AL5::Color(0, 0, 0));
		text_ip = "IP";
	}
	LL_AL5::Text text_port;
	{
		text_port.set_font(&agency_font);
		text_port.set_pos(10, 360);
		text_port.set_color(LL_AL5::Color(0, 0, 0));
		text_port = "Puerto";
	}
	LL_AL5::Text text_conn;
	{
		text_conn.set_font(&agency_font);
		text_conn.set_pos(520, 340);
		text_conn.set_color(LL_AL5::Color(0, 0, 0));
		text_conn = "Tiempo Conn.";
	}
	LL_AL5::Text text_send;
	{
		text_send.set_font(&agency_font);
		text_send.set_pos(520, 360);
		text_send.set_color(LL_AL5::Color(0, 0, 0));
		text_send = "Tiempo Pack.";
	}
	// Aplicación
	while (!input.get_display_status())
	{
		if (timer_get_connection.get_time() > time_get_connection)
		{
			timer_get_connection.clear();
			network.update_user();
		}
		if (timer_send_package.get_time() > time_send_package)
		{
			timer_send_package.clear();
			std::list<float> data_to_send;
			leap_motion.get_data(data_to_send);
			network.sendListData(data_to_send);
		}
		if (input.get_event())
		{
			if (server_button.is_clicked())
			{
				if (network.server_enable)
				{
					network.stop_server();
					server_button.set_button_text("Iniciar");
					textbox_ip.textbox_on();
					textbox_port.textbox_on();
				}
				else
				{
					std::string value = network.start_server(textbox_ip.get_value(), textbox_port.get_value());
					text_error_message = value;
					if (!value.size())
					{
						server_button.set_button_text("Detener");
						textbox_ip.textbox_off();
						textbox_port.textbox_off();
					}
				}
			}
			if (change_button.is_clicked())
			{
				std::string message = "";
				float new_time_conn = LL::to_float(textbox_conn.get_value());
				if (new_time_conn <= 0)
				{
					message = "Valor Ignorado Conn: " + LL::to_string(new_time_conn);
					textbox_conn.set_value(LL::to_string(time_get_connection));
					new_time_conn = time_get_connection;
				}
				float new_time_send = LL::to_float(textbox_send.get_value());
				if (new_time_send <= 0)
				{
					message = "Valor Ignorado Send: " + LL::to_string(new_time_send);
					textbox_send.set_value(LL::to_string(time_send_package));
					new_time_send = time_send_package;
				}
				time_get_connection = new_time_conn;
				time_send_package = new_time_send;
				if (message.size())
					text_error_message = message;
			}
		}
		display.clear();
		leap_motion.draw(&display);
		network.draw(&display);
		display.draw(&text_ip);
		display.draw(&text_port);
		display.draw(&textbox_ip);
		display.draw(&textbox_port);
		display.draw(&server_button);
		display.draw(&text_conn);
		display.draw(&text_send);
		display.draw(&textbox_conn);
		display.draw(&textbox_send);
		display.draw(&change_button);
		display.draw(&text_error_message);
		display.refresh();
	}
	return 0;
}