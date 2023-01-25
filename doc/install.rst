************
Installation
************

Dependencies
============

This package depends on a CMake-wrapped version of the Vicon Datastream SDK, which is
avialable `on GitHub
<https://github.com/intelligent-soft-robots/vicon-datastream-sdk>`_). For further
dependencies, see the ``find_package``-section in ``CMakeLists.txt``.

We also provide an Apptainer_ image ("pam_base") which contains all required
dependencies, so you don't need to install everything on your computer.
See pam_singularity_.


Build
=====

We recommend using treep_ to setup a workspace to build this package:

To install treep:

.. code-block:: bash

    pip install treep

Create workspace:

.. code-block:: bash

    mkdir ~/my_workspace
    cd ~/my_workspace
    git clone https://github.com/intelligent-soft-robots/treep_isr.git
    treep --clone-https PAM_VICON  # clones packages to ./workspace/src

Build with Apptainer:

.. code-block:: bash

    cd ~/my_workspace/workspace/
    apptainer shell -e path/to/container.sif
    Apptainer> colcon build

If you don't want to use Apptainer, simply call ``colcon build`` directly.  In this case,
you need to install all dependencies locally (see
:doc:`pam_documentation:A1_overview_and_installation`).

.. (see [PAM documentation](https://intelligent-soft-robots.github.io/pam_documentation/A1_overview_and_installation.html#dependencies-and-configuration-folder)).


.. _Apptainer: https://apptainer.org
.. _pam_singularity: https://github.com/intelligent-soft-robots/pam_singularity
.. _treep: https://pypi.org/project/treep/
