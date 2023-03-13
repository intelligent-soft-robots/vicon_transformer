
``vicon_transformer`` provides a wrapper around the `Vicon Datastream SDK`_ that
simplifies setup and access to the data.

Most relevantly, the :cpp:class:`~vicon_transformer::ViconTransformer` class can
provide poses of all subjects relative to a configurable "origin subject".  This
origin subject should be fixed in the scene (e.g. screwed to a wall), such that
it cannot move over time.  It then serves as a clearly defined origin of the
scene, that is independent of the actual origin used by Vicon (which otherwise
would need to be redefined after every re-calibration and is generally difficult
to set precisely).

The implementation is kept generic, such that it should work for different
applications.

The package also contains templated :ref:`o80 driver and standalone classes
<overview_o80>`, which can be specialised for specific applications by
configuring the list of subjects.

.. todo:: shorten last paragraph here and move to configure page


.. toctree::
   :caption: General Documentation
   :maxdepth: 1

   doc/install.rst
   doc/overview.rst
   doc/examples.rst


.. _Vicon Datastream SDK: https://www.vicon.com/software/datastream-sdk
.. _o80: https://github.com/intelligent-soft-robots/o80
