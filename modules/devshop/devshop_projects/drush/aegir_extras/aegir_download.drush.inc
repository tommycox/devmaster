<?php

use Symfony\Component\Process\Process;
use GitWrapper\GitWrapper;
use GitWrapper\GitWorkingCopy;
use GitWrapper\GitException;

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function aegir_download_drush_command()
{
    $items['provision-download'] = array(
        'description' => 'Downloads drupal modules and themes, and optionally commits them to git.',
        'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
        'options' => array(
            'packages' => 'This list of modules and themes to download.',
            'commit' => 'Commit the downloaded code to git.',
            'message' => 'The message to use when committing.',
            'update' => 'Run update.php after the download.',
            'test' => 'Queue a test run after the download.',
        ),
        'arguments' => array(
        ),
        'examples' => array(
            'drush @env.project.domain.com provision-download views ctools --update --commit --message="the usuals"' => 'Downloads views & ctools, commits them to the repo with a message, and run drush updb.',
        ),
        'aliases' => array('pdl'),
    );
    return $items;
}

/**
 * Implements the provision-git-pull command.
 */
function drush_aegir_download_provision_download() {
    $packages = drush_get_option('packages');

    if (is_array($packages)) {
        $packages = implode($packages);
    }

    // No modules, no more.
    if (empty($packages)) {
        return drush_set_error('No packages defined.');
    }

    // Run `drush dl $modules`
    $target = d()->name;
    $cmd = "drush $target dl $packages --yes --strict=0";

    // Pass through options to the drush dl command.
    if (drush_get_option('commit', FALSE)) {
      $cmd .= ' --commit ';
    }
    if (drush_get_option('message', '')) {
      $message = drush_get_option('message', '');
      $cmd .= " --message='$message' ";
    }
    if (drush_get_option('update', FALSE)) {
      $cmd .= " --update";
    }

    drush_log($cmd, 'devshop_command');

    $process = new Process($cmd);
    $process->setTimeout(null);
    $exit_code = $process->run(
      function ($type, $buffer) {
        if (Process::ERR === $type) {
          drush_log($buffer, 'devshop_ok');
        } else {
          drush_log($buffer, 'devshop_ok');
        }
      }
    );

    // check exit code
    if ($exit_code === 0) {
      drush_log("Aegir Download: OK", 'ok');
    } else {
      drush_log("Aegir Download Failed!", 'error');
      drush_log(dt('Downloading modules failed. Check the logs and try again.'), 'devshop_error');
      return;
    }
}

/**
 * Implements hook_drush_pm_post_download()
 *
 * Runs after a project has been downloaded.
 *
 * This is needed for devshop because we want to commit the new module we just
 * downloaded, and only this hook knows about the path.
 *
 * @param $drupal_project
 * @param $release
 */
function aegir_download_drush_pm_post_download($drupal_project, $release) {

  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->environment];

  if (d()->environment && drush_get_option('commit', FALSE)) {
    drush_log(dt('[DEVSHOP] Committing new module...'), 'ok');

    $message = drush_get_option('message', dt('Committed by DevShop: ') . $drupal_project['name']);

    $wrapper = new GitWrapper();
    $wrapper->streamOutput();

    try {
      $git = $wrapper->workingCopy($environment->repo_root);

      $download_path = $drupal_project['full_project_path'];
      $git
        ->add($download_path)
        ->commit($message)
        ->push();
    }
    catch (GitException $e) {
      return drush_set_error('DRUSH_ERROR', "Git Exception: " . $e->getMessage());
    }
  }
  else {
    drush_log(dt('Skipping Commit. Make sure you commit or delete this module at some point.'), 'notice');
  }
}
