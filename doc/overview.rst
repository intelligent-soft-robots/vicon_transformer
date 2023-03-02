********
Overview
********

Receiver
========

There are different receiver classes that provide
:cpp:class:`~vicon_transformer::ViconFrame` data from different sources:

- :cpp:class:`~vicon_transformer::ViconReceiver`: Uses the Vicon Datastream SDK to connect to
  a compatible Vicon application (e.g. Vicon Tracker).
- :cpp:class:`~vicon_transformer::PlaybackReceiver`: Loads a previously recorded Vicon frames
  from a file and plays them back.
- :cpp:class:`~vicon_transformer::JsonReceiver`:  Loads a single frame from a JSON file and
  always provides the data of that frame.  This is only meant for testing purposes.

ViconTransformer
================

:cpp:class:`~vicon_transformer::ViconTransformer`

Gets Vicon frames from a receiver and transforms subject poses such that they are
relative to a given "origin subject" (instead of relative to Vicon's origin).

Basic example in C++:

.. code-block:: C++

    #include <vicon_transformer/vicon_receiver.hpp>

    using vicon_transformer;

    int main()
    {
        // config is initialised with default values
        ViconReceiverConfig config;

        auto receiver = std::make_unique<ViconReceiver>(
            "vicon_pc_hostname", config);
        receiver->connect();  // connect to Vicon server

        ViconTransformer vt(receiver,  "my_origin_subject");

        // get new frame from receiver
        vt.update();

        if (vt.is_visible("robot_arm_marker"))
        {
            Transformation robot_tf = vt.get_transform("robot_arm_marker");
            // ...
        }
    }


Basic example in Python:

.. code-block:: Python

    from vicon_transformer import ViconReceiverConfig, ViconReceiver, ViconTransformer

    def main():
        config = ViconReceiverConfig()

        # instead of using a context manager, you can also call `receiver.connect()`
        with ViconReceiver("vicon_pc_hostname", config) as receiver:
            vt = ViconTransformer(receiver, "my_origin_subject")

            # get new frame from receiver
            vt.update()

            if vt.is_visible("robot_arm_marker"):
                robot_tf = vt.get_transform("robot_arm_marker")
                ...



o80 Driver/Standalone
=====================

This package provides templated driver and standalone classes to integrate
:cpp:class:`~vicon_transformer::ViconTransformer` through o80_.  Since o80 requires
observation data structures to be of fixed size, they need to be templated with the
number of subjects and a function that maps subject name to an index.  For more details
see the API documentation:

- :cpp:class:`~vicon_transformer::o80Driver`
- :cpp:class:`~vicon_transformer::o80Standalone`


Executables and Scripts
=======================

vicon_record
------------

Record data from a running Vicon system and safe to a file.  The file can then be played
back using the :cpp:class`~vicon_transformer::PlaybackReceiver`.

::

    vicon_record <hostname or IP> output_file.dat -d <duration in seconds>


vicon_print_data
----------------

Print data from either a running Vicon system or a previously recorded file:

::

    vicon_print_data <host or file>

See ``--help`` for all options.

.. important::

   ``vicon_print_data`` prints the poses as reported by the Vicon system, i.e. it
   does not transform with respect to some origin subject!


vicon_print_data_py
-------------------

Python-version of ``vicon_print_data`` (see above).  It doesn't support playback of
recorded files, apart from that it is mostly equivalent to the C++-version and mainly
serves as an example on how to use the Python bindings.


.. _o80: https://github.com/intelligent-soft-robots/o80
