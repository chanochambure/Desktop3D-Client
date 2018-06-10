#include <iostream>

#include "LexRisLogic\include\LexRisLogic\Allegro5\Allegro5.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Color.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Display.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Input.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Text.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Primitives.h"
#include "LexRisLogic\include\LexRisLogic\Allegro5\Special\Interface.h"
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
	float time_send_package = 0.5;
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
		text_error_message.set_pos(150, 100);
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
		}
		display.clear();
		leap_motion.draw(&display);
		network.draw(&display);
		display.draw(&text_ip);
		display.draw(&text_port);
		display.draw(&textbox_ip);
		display.draw(&textbox_port);
		display.draw(&server_button);
		display.draw(&text_error_message);
		display.refresh();
	}
	return 0;
}