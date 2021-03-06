<?php

use Symfony\Component\Process\Process;

/**
 * A Process class:
 */
class Provision_Service_Process extends Provision_Service {
}

/**
 * A Process class:
 */
class Provision_Service_Process_Process extends Provision_Service_Process {
  public $service = 'Process';

  function process($command, $cwd = null, $label = 'Process', $env = array()) {

    if (provision_is_local_host($this->server->remote_host)) {
      return devshop_drush_process($command, $cwd, $label, $env);
    }
    else {
      return devshop_drush_process('ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' ' . $this->server->script_user . '@' . $this->server->remote_host . ' ' . $command, $cwd, $label, $env);
    }
  }
}
