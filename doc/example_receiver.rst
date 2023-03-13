***********************************************
Example: Use ViconReceiver to Access Vicon Data
***********************************************

The API for accessing the Vicon data is implemented in C++ but bindings are generated,
so it can also be used with Python.

Below are examples in both Python and C++.  Note that they only use the low level
receiver class which gives poses as provided by Vicon and does not transform them to a
specific reference frame.


.. _example_receiver_python:

Python
======

The script ``vicon_print_data_py.py`` shows how to set up a
:cpp:class:`~vicon_transformer::ViconReceiver`
instance to get the object information provided by Vicon and print it to stdout.

.. literalinclude:: /PKG/scripts/vicon_print_data_py.py
   :language: python


C++
===

Below is the code of the ``vicon_print_data`` executable implemented in C++.  It is
mostly equivalent to the Python implementation above but also supports using a
:cpp:class:`~vicon_transformer::PlaybackReceiver` to play back a previously recorded
file instead of connecting to a live Vicon system.

.. literalinclude:: /PKG/src/vicon_print_data.cpp
   :language: c++
