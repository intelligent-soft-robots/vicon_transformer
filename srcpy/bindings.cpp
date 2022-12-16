/**
 * @file Python bindings of the relevant C++ classes/functions.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/vicon_receiver.hpp>

PYBIND11_MODULE(vicon_transformer_bindings, m)
{
    namespace py = pybind11;
    namespace vt = vicon_transformer;

    py::register_exception<vt::NotConnectedError>(
        m, "NotConnectedError", PyExc_RuntimeError);
    py::register_exception<vt::BadResultError>(
        m, "BadResultError", PyExc_RuntimeError);

    py::class_<vt::SubjectData>(m, "SubjectData")
        .def(py::init<>())
        .def_readwrite("is_visible", &vt::SubjectData::is_visible)
        .def_readwrite("global_translation",
                       &vt::SubjectData::global_translation)
        .def_readwrite("global_rotation_quaternion",
                       &vt::SubjectData::global_rotation_quaternion)
        .def_readwrite("quality", &vt::SubjectData::quality);

    py::class_<vt::ViconFrame>(m, "ViconFrame")
        .def(py::init<>())
        .def_readwrite("frame_number", &vt::ViconFrame::frame_number)
        .def_readwrite("frame_rate", &vt::ViconFrame::frame_rate)
        .def_readwrite("latency", &vt::ViconFrame::latency)
        .def_readwrite("time_stamp", &vt::ViconFrame::time_stamp)
        .def_readwrite("subjects", &vt::ViconFrame::subjects)
        .def("__str__",
             [](const vt::ViconFrame& vf)
             {
                 std::stringstream stream;
                 stream << vf;
                 return stream.str();
             });

    py::class_<vt::ViconReceiverConfig>(m, "ViconReceiverConfig")
        .def(py::init<>())
        .def_readwrite("enable_lightweight",
                       &vt::ViconReceiverConfig::enable_lightweight)
        .def_readwrite("buffer_size", &vt::ViconReceiverConfig::buffer_size);

    py::class_<vt::ViconReceiver>(m, "ViconReceiver")
        .def(py::init<std::string, vt::ViconReceiverConfig>(),
             py::arg("host_name"),
             py::arg("config"))
        .def("is_connected", &vt::ViconReceiver::is_connected)
        .def("connect", &vt::ViconReceiver::connect)
        .def("disconnect", &vt::ViconReceiver::disconnect)
        .def("read", &vt::ViconReceiver::read)
        .def("print_info", &vt::ViconReceiver::print_info)
        .def("print_latency_info", &vt::ViconReceiver::print_latency_info)
        .def("filter_subjects", &vt::ViconReceiver::filter_subjects);
}
