/**
 * @file Python bindings of the relevant C++ classes/functions.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
// TODO: This should be moved to a separate package
#include <sstream>

#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <o80/pybind11_helper.hpp>
#include <serialization_utils/cereal_json.hpp>

#include <vicon_transformer/pam_vicon_o80.hpp>

PYBIND11_MODULE(pam_vicon_o80, m)
{
    namespace py = pybind11;

    py::module::import("vicon_transformer");

    py::class_<pam_vicon_o80::FixedSizeViconFrame>(m, "FixedSizeViconFrame")
        .def(py::init<>())
        .def_readwrite("frame_number",
                       &pam_vicon_o80::FixedSizeViconFrame::frame_number)
        .def_readwrite("frame_rate",
                       &pam_vicon_o80::FixedSizeViconFrame::frame_rate)
        .def_readwrite("latency", &pam_vicon_o80::FixedSizeViconFrame::latency)
        .def_readwrite("time_stamp",
                       &pam_vicon_o80::FixedSizeViconFrame::time_stamp)
        .def_readwrite("subjects",
                       &pam_vicon_o80::FixedSizeViconFrame::subjects)
        .def(
            "__str__",
            [](const pam_vicon_o80::FixedSizeViconFrame& vf) {
                std::stringstream stream;
                stream << vf;
                return stream.str();
            },
            py::call_guard<py::gil_scoped_release>());
    m.def("to_json",
          &serialization_utils::to_json<pam_vicon_o80::FixedSizeViconFrame>);
    m.def("from_json",
          &serialization_utils::from_json<pam_vicon_o80::FixedSizeViconFrame>);

    py::enum_<pam_vicon_o80::Subjects>(m, "Subjects")
        .value("BALL_LAUNCHER", pam_vicon_o80::Subjects::BALL_LAUNCHER)
        .value("PING_BASE", pam_vicon_o80::Subjects::PING_BASE)
        .value("ARM", pam_vicon_o80::Subjects::ARM)
        .value("TABLE_CORNER_1", pam_vicon_o80::Subjects::TABLE_CORNER_1)
        .value("TABLE_CORNER_2", pam_vicon_o80::Subjects::TABLE_CORNER_2)
        .value("TABLE_CORNER_3", pam_vicon_o80::Subjects::TABLE_CORNER_3)
        .value("TABLE_CORNER_4", pam_vicon_o80::Subjects::TABLE_CORNER_4)
        .value("LED_STICK", pam_vicon_o80::Subjects::LED_STICK)
        .value("MUSCLE_BASE", pam_vicon_o80::Subjects::MUSCLE_BASE)
        .value("MUSCLE_RACKET", pam_vicon_o80::Subjects::MUSCLE_RACKET);

    m.def("map_subject_name_to_index",
          &pam_vicon_o80::map_subject_name_to_index);
    m.def("get_subject_names", &pam_vicon_o80::get_subject_names);

    o80::create_python_bindings<pam_vicon_o80::o80Standalone,
                                o80::NO_EXTENDED_STATE>(m);
    o80::create_standalone_python_bindings<
        pam_vicon_o80::o80Driver,
        pam_vicon_o80::o80Standalone,
        // argument for the driver
        std::shared_ptr<vicon_transformer::Receiver>,
        // argument for the driver (origin subject name)
        const std::string&>(m);
}
