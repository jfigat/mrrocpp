/* Y o u r   D e s c r i p t i o n                       */
/*                            AppBuilder Photon Code Lib */
/*                                         Version 2.01  */

#include "ui_r_sarkofag.h"
#include "../base/ui_ecp_robot/ui_ecp_r_common.h"
#include "robot/sarkofag/const_sarkofag.h"
#include "../base/interface.h"

#include "../base/mainwindow.h"
#include "ui_mainwindow.h"

#include "../base/wgt_single_motor_move.h"

namespace mrrocpp {
namespace ui {
namespace sarkofag {
const std::string WGT_SARKOFAG_MOVE = "WGT_SARKOFAG_MOVE";
//
//
// KLASA UiRobot
//
//

void UiRobot::edp_create()
{
	if (state.edp.state == 0) {
		create_thread();

		eb.command(boost::bind(&ui::sarkofag::UiRobot::edp_create_int, &(*this)));
	}
}

int UiRobot::edp_create_int()

{
	interface.set_ui_state_notification(UI_N_PROCESS_CREATION);

	try { // dla bledow robot :: ECP_error

		// dla robota sarkofag
		if (state.edp.state == 0) {

			state.edp.state = 0;
			state.edp.is_synchronised = false;

			std::string tmp_string("/dev/name/global/");
			tmp_string += state.edp.hardware_busy_attach_point;

			std::string tmp2_string("/dev/name/global/");
			tmp2_string += state.edp.network_resourceman_attach_point;

			// sprawdzenie czy nie jest juz zarejestrowany zarzadca zasobow
			if (((!(state.edp.test_mode)) && (access(tmp_string.c_str(), R_OK) == 0))
					|| (access(tmp2_string.c_str(), R_OK) == 0)) {
				interface.ui_msg->message(lib::NON_FATAL_ERROR, "edp_sarkofag already exists");
			} else if (interface.check_node_existence(state.edp.node_name, "edp_sarkofag")) {

				state.edp.node_nr = interface.config->return_node_number(state.edp.node_name);

				{
					boost::unique_lock <boost::mutex> lock(interface.process_creation_mtx);

					ui_ecp_robot = new ui::common::EcpRobot(interface, lib::sarkofag::ROBOT_NAME);
				}

				state.edp.pid = ui_ecp_robot->ecp->get_EDP_pid();

				if (state.edp.pid < 0) {

					state.edp.state = 0;
					fprintf(stderr, "edp spawn failed: %s\n", strerror(errno));
					delete ui_ecp_robot;
				} else { // jesli spawn sie powiodl

					state.edp.state = 1;

					connect_to_reader();

					// odczytanie poczatkowego stanu robota (komunikuje sie z EDP)
					lib::controller_state_t robot_controller_initial_state_tmp;

					ui_ecp_robot->get_controller_state(robot_controller_initial_state_tmp);

					//state.edp.state = 1; // edp wlaczone reader czeka na start

					state.edp.is_synchronised = robot_controller_initial_state_tmp.is_synchronised;
				}
			}
		}

	} // end try

	CATCH_SECTION_UI

	interface.manage_interface();
	wgt_move -> synchro_depended_init();
	return 1;

}

int UiRobot::execute_motor_motion()
{
	try {

		ui_ecp_robot->move_motors(desired_pos);

	} // end try
	CATCH_SECTION_UI

	return 1;
}

int UiRobot::execute_joint_motion()
{
	try {

		ui_ecp_robot->move_joints(desired_pos);

	} // end try
	CATCH_SECTION_UI

	return 1;
}

int UiRobot::synchronise()

{

	eb.command(boost::bind(&ui::sarkofag::UiRobot::synchronise_int, &(*this)));

	return 1;

}

int UiRobot::synchronise_int()

{

	interface.set_ui_state_notification(UI_N_SYNCHRONISATION);

	// wychwytania ew. bledow ECP::robot
	try {
		// dla robota irp6_on_track

		if ((state.edp.state > 0) && (state.edp.is_synchronised == false)) {
			ui_ecp_robot->ecp->synchronise();
			state.edp.is_synchronised = ui_ecp_robot->ecp->is_synchronised();
		} else {
			// 	printf("edp irp6_on_track niepowolane, synchronizacja niedozwolona\n");
		}

	} // end try
	CATCH_SECTION_UI

	// modyfikacje menu
	interface.manage_interface();
	wgt_move->synchro_depended_init();
	return 1;

}

UiRobot::UiRobot(common::Interface& _interface) :
			single_motor::UiRobot(_interface, lib::sarkofag::EDP_SECTION, lib::sarkofag::ECP_SECTION, lib::sarkofag::ROBOT_NAME, lib::sarkofag::NUM_OF_SERVOS, "is_sarkofag_active")
{

	wgt_move = new wgt_single_motor_move("Sarkofag moves", interface, *this, interface.get_main_window());
	wndbase_m[WGT_SARKOFAG_MOVE] = wgt_move->dwgt;

}

int UiRobot::manage_interface()
{
	MainWindow *mw = interface.get_main_window();
	Ui::MainWindow *ui = mw->get_ui();

	switch (state.edp.state)
	{
		case -1:
			mw->enable_menu_item(false, 1, ui->menuSarkofag);

			break;
		case 0:
			mw->enable_menu_item(false, 4, ui->actionsarkofag_EDP_Unload, ui->actionsarkofag_Synchronisation, ui->actionsarkofag_Move, ui->actionsarkofag_Servo_Algorithm);
			mw->enable_menu_item(true, 1, ui->menuSarkofag);
			mw->enable_menu_item(true, 1, ui->actionsarkofag_EDP_Load);
			mw->enable_menu_item(false, 1, ui->menusarkofag_Preset_Positions);

			break;
		case 1:
		case 2:
			mw->enable_menu_item(true, 1, ui->menuSarkofag);

			// jesli robot jest zsynchronizowany
			if (state.edp.is_synchronised) {
				mw->enable_menu_item(false, 1, ui->actionsarkofag_Synchronisation);
				mw->enable_menu_item(true, 1, ui->menuall_Preset_Positions);

				switch (interface.mp.state)
				{
					case common::UI_MP_NOT_PERMITED_TO_RUN:
					case common::UI_MP_PERMITED_TO_RUN:
						mw->enable_menu_item(true, 3, ui->actionsarkofag_EDP_Unload, ui->actionsarkofag_Move, ui->actionsarkofag_Servo_Algorithm);
						mw->enable_menu_item(false, 1, ui->actionsarkofag_EDP_Load);
						mw->enable_menu_item(true, 1, ui->menusarkofag_Preset_Positions);

						break;
					case common::UI_MP_WAITING_FOR_START_PULSE:
						mw->enable_menu_item(true, 2, ui->actionsarkofag_Move, ui->actionsarkofag_Servo_Algorithm);
						mw->enable_menu_item(false, 2, ui->actionsarkofag_EDP_Load, ui->actionsarkofag_EDP_Unload);
						mw->enable_menu_item(true, 1, ui->menusarkofag_Preset_Positions);

						break;
					case common::UI_MP_TASK_RUNNING:
					case common::UI_MP_TASK_PAUSED:
						mw->enable_menu_item(false, 2, ui->actionsarkofag_Move, ui->actionsarkofag_Servo_Algorithm);
						mw->enable_menu_item(false, 1, ui->menusarkofag_Preset_Positions);

						break;
					default:
						break;
				}
			} else // jesli robot jest niezsynchronizowany
			{
				mw->enable_menu_item(true, 4, ui->actionsarkofag_EDP_Unload, ui->actionsarkofag_Synchronisation, ui->actionsarkofag_Move, ui->actionall_Synchronisation);
				mw->enable_menu_item(false, 1, ui->actionsarkofag_EDP_Load);

			}
			break;
		default:
			break;
	}

	return 1;
}

}
} //namespace ui
} //namespace mrrocpp

