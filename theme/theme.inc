<?php

function so_forms_preprocess_so_form(&$vars) {   
    
    $vars['form_is_populated'] = !empty($vars['form']['#form_is_populated']);
    $vars['advanced_form_is_populated'] = !empty($vars['form']['#advanced_form_is_populated']);

    $vars['form_name'] = $vars['form']['#public_infos']['name'];
    $vars['secured_sfid'] = $vars['form']['#public_infos']['secured_sfid'];

    $vars['normal_fields'] = drupal_render($vars['form']['normal']);
    $vars['advanced_fields'] = drupal_render($vars['form']['advanced']);

    $vars['submit'] = drupal_render($vars['form']['submit']);
    $vars['reset'] = drupal_render($vars['form']['reset']);

    $vars['form_params'] = drupal_render($vars['form']);
}