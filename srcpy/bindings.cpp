/**
 * @file Python bindings of the relevant C++ classes/functions.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <sstream>

#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <o80/pybind11_helper.hpp>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/o80_driver.hpp>
#include <vicon_transformer/o80_standalone.hpp>
#include <vicon_transformer/transform.hpp>
#include <vicon_transformer/vicon_receiver.hpp>

#include <vicon_transformer/pam_vicon_o80.hpp>

PYBIND11_MODULE(vicon_transformer_bindings, m)
{
    namespace py = pybind11;
    namespace vt = vicon_transformer;

    py::register_exception<vt::NotConnectedError>(
        m, "NotConnectedError", PyExc_RuntimeError);
    py::register_exception<vt::BadResultError>(
        m, "BadResultError", PyExc_RuntimeError);

    // TODO: add unit tests for bindings of Transformation
    py::class_<vt::Transformation>(m, "Transformation")
        .def(py::init<>(), py::call_guard<py::gil_scoped_release>())
        .def(py::init<Eigen::Quaterniond, Eigen::Vector3d>(),
             py::call_guard<py::gil_scoped_release>())
        .def(py::self * py::self)
        .def(py::self * Eigen::Vector3d())
        .def("apply",
             &vt::Transformation::apply,
             py::call_guard<py::gil_scoped_release>())
        .def("inverse",
             &vt::Transformation::inverse,
             py::call_guard<py::gil_scoped_release>())
        .def("matrix",
             &vt::Transformation::matrix,
             py::call_guard<py::gil_scoped_release>())
        .def_readwrite("translation", &vt::Transformation::translation)
        // there are no proper bindings for Eigen::Quaterniond, so as a simple
        // workaround provide a getter and setter that provide/expect the
        // quaternion as a list [x, y, z, w].
        .def("get_rotation",
             [](const vt::Transformation& self) {
                 std::array<double, 4> quat = {self.rotation.x(),
                                               self.rotation.y(),
                                               self.rotation.z(),
                                               self.rotation.w()};
                 return quat;
             })
        .def("set_rotation",
             [](vt::Transformation& self, const std::array<double, 4>& quat) {
                 self.rotation.x() = quat[0];
                 self.rotation.y() = quat[1];
                 self.rotation.z() = quat[2];
                 self.rotation.w() = quat[3];
             });

    py::class_<vt::SubjectData>(m, "SubjectData")
        .def(py::init<>())
        .def_readwrite("is_visible", &vt::SubjectData::is_visible)
        .def_readwrite("global_pose", &vt::SubjectData::global_pose)
        .def_readwrite("quality", &vt::SubjectData::quality);
    m.def("to_json", &vt::to_json<vt::SubjectData>);
    m.def("from_json", &vt::from_json<vt::SubjectData>);

    py::class_<vt::ViconFrame>(m, "ViconFrame")
        .def(py::init<>())
        .def_readwrite("frame_number", &vt::ViconFrame::frame_number)
        .def_readwrite("frame_rate", &vt::ViconFrame::frame_rate)
        .def_readwrite("latency", &vt::ViconFrame::latency)
        .def_readwrite("time_stamp", &vt::ViconFrame::time_stamp)
        .def_readwrite("subjects", &vt::ViconFrame::subjects)
        .def(
            "__str__",
            [](const vt::ViconFrame& vf) {
                std::stringstream stream;
                stream << vf;
                return stream.str();
            },
            py::call_guard<py::gil_scoped_release>());
    m.def("to_json", &vt::to_json<vt::ViconFrame>);
    m.def("from_json", &vt::from_json<vt::ViconFrame>);

    py::class_<vt::ViconReceiverConfig>(m, "ViconReceiverConfig")
        .def(py::init<>())
        .def_readwrite("enable_lightweight",
                       &vt::ViconReceiverConfig::enable_lightweight)
        .def_readwrite("buffer_size", &vt::ViconReceiverConfig::buffer_size)
        .def_readwrite("filtered_subjects",
                       &vt::ViconReceiverConfig::filtered_subjects);
    m.def("to_json", &vt::to_json<vt::ViconReceiverConfig>);
    m.def("from_json", &vt::from_json<vt::ViconReceiverConfig>);

    py::class_<vt::Receiver, std::shared_ptr<vt::Receiver>> PyReceiver(
        m, "Receiver");
    py::class_<vt::ViconReceiver,
               std::shared_ptr<vt::ViconReceiver>,
               vt::Receiver>(m, "ViconReceiver")
        .def(py::init<std::string, vt::ViconReceiverConfig>(),
             py::arg("host_name"),
             py::arg("config"),
             py::call_guard<py::gil_scoped_release>())
        .def("is_connected",
             &vt::ViconReceiver::is_connected,
             py::call_guard<py::gil_scoped_release>())
        .def("connect",
             &vt::ViconReceiver::connect,
             py::call_guard<py::gil_scoped_release>())
        .def("disconnect",
             &vt::ViconReceiver::disconnect,
             py::call_guard<py::gil_scoped_release>())
        .def("read",
             &vt::ViconReceiver::read,
             py::call_guard<py::gil_scoped_release>())
        .def("print_info",
             &vt::ViconReceiver::print_info,
             py::call_guard<py::gil_scoped_release>())
        .def("print_latency_info",
             &vt::ViconReceiver::print_latency_info,
             py::call_guard<py::gil_scoped_release>());
    py::class_<vt::PlaybackReceiver,
               std::shared_ptr<vt::PlaybackReceiver>,
               vt::Receiver>(m, "PlaybackReceiver")
        .def(py::init<const std::filesystem::path&>(),
             py::arg("filename"),
             py::call_guard<py::gil_scoped_release>())
        .def("read",
             &vt::PlaybackReceiver::read,
             py::call_guard<py::gil_scoped_release>());

    // -----------------------------------------
    // TODO: Bindings below are application-specific and should be moved to a
    // separate package

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
    m.def("to_json", &vt::to_json<pam_vicon_o80::FixedSizeViconFrame>);
    m.def("from_json", &vt::from_json<pam_vicon_o80::FixedSizeViconFrame>);

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
