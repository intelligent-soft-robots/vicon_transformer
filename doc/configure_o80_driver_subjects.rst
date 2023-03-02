.. _configure_subjects_o80:

*********************************
Configure Subjects for o80 Driver
*********************************

While the :cpp:class:`vicon_transformer::ViconFrame` instances returned by
:cpp:class:`vicon_transformer::ViconReceiver` contain a variable-sized map with all
subjects that are reported by the Vicon Tracker software, this is can unfortunately not
be used with o80 as the inter-process communication via shared memory requires that data
types are of fixed size.  This does not work go with a variable amount of subjects, each
having a name of variable length.

Instead, the o80 driver provides the data as
:cpp:class:`vicon_transformer::FixedSizeViconFrame`.  This has two consequences:

1. The number of subjects has to be known at compile time.
2. Subject names are not included in the data structure.  Instead, their poses are given
   in an array in an order that has to be specified at compile time.

In practice this is done by defining an enum :cpp:enum:`pam_vicon_o80::Subjects` whose
values serve as indices for the subject array.  That is, to get the data of the robot
base subject, use code like this (using Python bindings, corresponding C++ code will
look similar):

.. code-block:: Python

   frame = frontend.latest().get_extended_state()
   robot_base = frame.subjects[Subjects.MUSCLE_BASE]
   print("Robot position: ", robot_base.global_pose.translation)


This means, however, that if something changes in the Vicon configuration (e.g. subjects
are added or removed or their name is changed), the code in ``pam_vicon_o80.hpp`` has to
be updated accordingly.  More specifically, the following changes need to be made:

- ``NUM_SUBJECTS``:  This constant specifies the number of subjects.
- ``enum Subjects``:  There needs to be an entry for each subject here.
- ``_subject_name_to_index``:  Maps the subject name (as used by Vicon) to an index in
  the array.  This is used inside the o80 driver to fill the subjects array in the
  correct order.

.. note::

   You don't necessarily need to list all subjects that are tracked by Vicon, it is
   enough to list only those that are needed in your code.  If Vicon reports additional
   subjects, whose name is not listed in ``_subject_name_to_index``, they will simply be
   ignored.

   Likewise, if ``_subject_name_to_index`` contains subject names for which Vicon does
   not provide any data (because the subject doesn't exist or is disabled), it will be
   handled by the driver as if it was not visible.


Example:

.. code-block:: C++

    constexpr std::size_t NUM_SUBJECTS = 7;

    enum Subjects
    {
        PING_BASE = 0,
        TABLE_CORNER_1,
        TABLE_CORNER_2,
        TABLE_CORNER_3,
        TABLE_CORNER_4,
        LED_STICK,
        MUSCLE_BASE,
    };

    const std::map<std::string, size_t> _subject_name_to_index = {
        {"rll_ping_base", PING_BASE},
        {"TT Platte_Eckteil 1", TABLE_CORNER_1},
        {"TT Platte_Eckteil 2", TABLE_CORNER_2},
        {"TT Platte_Eckteil 3", TABLE_CORNER_3},
        {"TT Platte_Eckteil 4", TABLE_CORNER_4},
        {"rll_led_stick", LED_STICK},
        {"rll_muscle_base", MUSCLE_BASE},
    };
