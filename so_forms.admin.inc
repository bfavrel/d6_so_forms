<?php

/**
 * Fields overview for a given form
 *
 * @param array $form_state
 * @param string $sfid
 * @param string $lang
 *
 * @return array
 */
function so_forms_fields_overview($form_state, $sfid, $lang = null) {

    $destination = drupal_get_destination();

    $form_params = so_forms_load_form_params($sfid);

    so_forms_load_fields_pool($sfid);
    $registered_fields_types = so_forms_registered_fields_types();
    $registered_widgets = so_forms_registered_widgets();

    // On met le cache des définitions de classes à jour.
    // On le fait à cet endroit, car il est inutile de rafraichir ce cache à chaque chargement du module.
    $cache = new stdClass();

    $cache->sfid = $sfid;
    $cache->params = serialize(array(
        'configuration' => $form_params->configuration,
        'definitions_cache' => array(
            'registered_fields_types' => $registered_fields_types,
            'registered_widgets' => $registered_widgets,
        ),
    ));
    drupal_write_record('so_forms', $cache, 'sfid');

    $form['#tree'] = true;

    $form['sfid'] = array(
        '#type' => 'hidden',
        '#value' => $sfid,
    );

    $form['fields'] = array();

    $form_groups = so_forms_get_stored_groups($sfid);

    foreach($form_groups as $key => &$form_group) {

        if($form_group->field_type == 'group') {
            $form['fields']['group_' . $form_group->id] = array(
                '#depth' => 0,
                '#weight' => $form_group->weight,
                '#is_group' => 1,
                'weight' => array(
                    '#type' => 'weight',
                    '#delta' => 50,
                    '#default_value' => $form_group->weight,
                ),
                'enabled' => array(
                    '#type' => 'checkbox',
                    '#default_value' => $form_group->params['enabled'],
                    '#return_value' => 1,
                ),
                'fid' => array(
                    '#type' => 'textfield',
                    '#default_value' => $form_group->id,
                ),
                'pfid' => array(
                    '#type' => 'textfield',
                    '#default_value' => null,
                ),
                'form_region' => array(
                    '#type' => 'textfield',
                    '#default_value' => $form_params->configuration['advanced'] == true ? $form_group->params['form_region'] : 'normal',
                ),
		'label' => array(
                    '#type' => 'textfield',
                    '#default_value' => html_entity_decode($form_group->label, ENT_QUOTES),
                    '#size' => 30,
                ),
                'operations' => array(
                    'delete' => array(
                        '#value' => l(t('delete'), "admin/so_forms/delete_group/" . $sfid . "/" . $form_group->id, array('query' => $destination)),
                    ),
                ),
            );
        }
    }

    $form_fields = so_forms_get_stored_fields($sfid);

    foreach($form_fields as $key => $form_field) {

        $field_name = explode(':', $form_field->field);

        $sub_form = array(
            '#depth' => (int)($form_field->gid != null),
            '#weight' => $form_field->weight,
            'weight' => array(
                '#type' => 'weight',
                '#delta' => 50,
                '#default_value' => $form_field->weight,
            ),
            'enabled' => array(
                '#type' => 'checkbox',
                '#default_value' => $form_field->params['enabled'],
                '#return_value' => 1,
            ),
            'fid' => array(
                '#type' => 'textfield',
                '#default_value' => $form_field->id,
            ),
            'pfid' => array(
                '#type' => 'textfield',
                '#default_value' => $form_field->gid,
            ),
            'form_region' => array(
                '#type' => 'textfield',
                '#default_value' => $form_params->configuration['advanced'] == true ? $form_field->params['form_region'] : 'normal',
            ),
            'label' => array(
                '#type' => 'textfield',
                '#default_value' => html_entity_decode($form_field->label, ENT_QUOTES),
                '#size' => 30,
                '#attributes' => array('title' => t("Original field : !field_name", array('!field_name' => $field_name[1]))),
            ),
            'field_infos' => array(
                '#type' => 'markup',
                '#value' => t($registered_fields_types[$form_field->field_type]['label']) . " : " . t($registered_widgets[$form_field->widget]['label']),
            ),
        );

        $field_instance = so_forms_get_field_instance($form_field->id);
        $field_config_form = $field_instance->fieldConfigurationForm(array());

        if(!empty($field_config_form)) {
            $sub_form['operations']['edit'] = array(
                '#value' => l(t("configure"), "admin/so_forms/edit_field/" . $sfid . "/" . $form_field->id . "/" . $lang, array('query' => $destination)),
            );
        }

        if($form_params->configuration['api_only'] != true) {
            $sub_form['operations']['delete'] = array(
                '#value' => l(t("delete"), "admin/so_forms/delete_field/" . $sfid . "/" . $form_field->id, array('query' => $destination)),
            );
        }

        if(!empty($form_field->gid)) {
            $form['fields']['group_' . $form_field->gid]['field_'. $form_field->id] = $sub_form;
        } else {
            $form['fields']['field_'. $form_field->id] = $sub_form;
        }
    }

    $weights = array();
    foreach ($form['fields'] as $key => $field) {
        $weights[$key] = $field['#weight'];
    }
    array_multisort($weights, SORT_ASC, $form['fields']);

    global $user;

    if($user->uid == 1) {
        $form['informations'] = array(
            '#type' => 'fieldset',
            '#title' => t("Informations"),
            '#collapsible' => true,
            '#collapsed' => true,
            '#tree' => true,
        );

        $sfid_infos = explode(':', $sfid); // 0 : module / 1 : id

        $form['informations']['includes'] = array(
            '#type' => 'markup',
            '#prefix' => "<div>",
            '#value' => "<strong>" . t("The resulting form can be displayed using")  . " : </strong><br />" .
                        "<ul>" .
                        "<li>so_form(null, '" . $form_params->secured_sfid . "')</li>" .
                        "<li>so_form('" . $sfid_infos[0] . "', '" . $sfid_infos[1] . "')</li>" .
                        "</ul>",
            '#suffix' => "</div>",
        );

        $form['informations']['templates'] = array(
            '#type' => 'markup',
            '#prefix' => "<div>",
            '#value' => "<strong>" . t("Templates")  . " : </strong><br />" .
                        "<ul>" .
                        "<li>so-form--" . $form_params->secured_sfid . ".tpl.php</li>" .
                        "<li>so-form--" . $sfid_infos[0] . "-" . $sfid_infos[1] . ".tpl.php</li>" .
                        "<li>so-form--" . $sfid_infos[0] . ".tpl.php</li>" .
                        "<li>so-form.tpl.php</li>" .
                        "</ul>",
            '#suffix' => "</div>",
        );
    }

    if(!empty($form['fields'])) {
        $form['submit'] = array(
            '#type' => 'submit',
            '#value' => t('Save'),
        );
    }

    $form['#validate'] = array('so_forms_fields_overview_validate');

    if($form_params->configuration['api_only'] != true) {

        $fields_options = so_forms_modules_fields_list($sfid);
        $fields_options = array_merge(array('' => t('-- Select a field --')), $fields_options);

        $form['add_field'] = array();
        $form['add_field']['#depth'] = 1;
        $form['add_field']['label'] = array(
            '#type' => 'textfield',
            '#title' => t("Label"),
            '#description' => t("If empty, field name will be used."),
            '#size' => 25,
        );

        $form['add_field']['field'] = array(
            '#type' => 'select',
            '#title' => t('Field'),
            '#options' => $fields_options,
            '#attributes' => array('style' => 'max-width: 200px;'),
            '#ahah' => array(
                'path' => 'admin/so_forms/js/field_type_options/' . $sfid,
                'wrapper' => 'field_type_wrapper',
                'effect' => 'fade',
                'event' => 'change',
                'method' => 'replace',
            ),
        );

        $form['add_field']['field_wrapper'] = array(
            '#type' => 'markup',
            '#prefix' => "<div id='field_type_wrapper'>",
            '#suffix' => "</div>",
        );

        $form['add_field']['field_wrapper']['field_type'] = array(
            '#type' => 'item',
            '#title' => t("Field type"),
            '#value' => t("Select a field"),

            '#attributes' => array('style' => 'max-width: 200px;'),
        );

        $form['add_field']['submit'] = array(
            '#type' => 'submit',
            '#value' => t('Add a field'),
            '#submit' => array('so_forms_add_field_submit'),
            '#op' => 'add_field',
        );
    }

    if($form_params->configuration['no_groups'] != true) {
        $form['add_group'] = array();
        $form['add_group']['#depth'] = 1;
        $form['add_group']['label'] = array(
            '#type' => 'textfield',
            '#title' => t("Title"),
            '#size' => 25,
        );
        $form['add_group']['submit'] = array(
            '#type' => 'submit',
            '#value' => t('Add a group'),
            '#submit' => array('so_forms_add_group_submit'),
            '#op' => 'add_group',
        );
    }

    return $form;
}

/**
 * Returns a list of fields provided by modules
 *
 * @param string $sfid
 * @param string/array $modules : optional : filter by modules
 * @param string/array $types : optional : filter by field types
 * @param string/array $contexts : optional : filter by field contexts
 * @param boolean $flat : should the list be grouped by providers ?
 *
 * @return array : fields grouped by modules or not
 */
function so_forms_modules_fields_list($sfid = null, $modules = array(), $types = array(), $contexts = array(), $flat = false) {

    $fields_pool = so_forms_load_fields_pool($sfid);
    $registered_fields_types = so_forms_registered_fields_types();

    $fields_types_list = array_keys($registered_fields_types);
    $fields = array();

    foreach($fields_pool as $key => $field) {

        $field_infos = explode(':', $key);

        if(!empty($modules)) {
            if(!in_array($field_infos[0], (array)$modules)) {
                continue;
            }
        }

        if(!empty($contexts)) {
            if(!in_array($field_infos[2], (array)$contexts)) {
                continue;
            }
        }

        if(empty($types)) {$types = $fields_types_list;}
        $field_types = array_intersect((array)$field['type'], $fields_types_list, (array)$types);

        if(empty($field_types)) {
            continue;
        }

        $fields[$key] = $field['label'];
    }

    if($flat == true) {
        return $fields;
    }

    $list = array();

    foreach($fields as $key => $field) {

        $module = explode(':', $key);
        $module = $module[0];

        $list[$module][$key] = $field;
    }

    return $list;
}

/**
 * Overview form validation.
 */
function so_forms_fields_overview_validate(&$form, &$form_state) {

    if ($form_state['clicked_button']['#op'] == 'add_field') {
        if(empty($form_state['values']['add_field']['label'])) {

            if($form_state['values']['add_field']['field'] != '') {
                $fields_list = so_forms_modules_fields_list($form_state['values']['sfid'], null, null, null, true);
                $form_state['values']['add_field']['label'] = $fields_list[$form_state['values']['add_field']['field']];
            } else {
                form_set_error('add_field][label', t('You have to set a label for this field.'));
            }
        }

        if($form_state['values']['add_field']['field'] == '') {
            form_set_error('add_field][field', t('You have to select a field.'));
        }

        if(empty($form_state['values']['add_field']['field_wrapper']['field_type'])) {
            form_set_error('add_field][field_wrapper][field_type', t('You have to select a field type.'));
        }

    } elseif ($form_state['clicked_button']['#op'] == 'add_group') {
        if ($form_state['values']['add_group']['label'] == '') {
            form_set_error('add_group][label', t('You have to set a title for this group.'));
        }
    }
}

/**
 * Form Submit. Add a field.
 */
function so_forms_add_field_submit($form, &$form_state) {

    $fields_pool = so_forms_load_fields_pool($form_state['values']['sfid']);

    $form_params = explode(':', $form_state['values']['sfid']);

    $field_type_infos = explode(':', $form_state['values']['add_field']['field_wrapper']['field_type']);

    $field_infos = explode(':', $form_state['values']['add_field']['field']);

    $field = array(
        'module' => $field_infos[0],
        'name' => $field_infos[1],
        'label' => $form_state['values']['add_field']['label'],
        'type' => $field_type_infos[0],
        'widget' => $field_type_infos[1],
        'context' => $field_infos[2],
        'callbacks' => $fields_pool[$form_state['values']['add_field']['field']]['callbacks'],
    );

    so_forms_add_field($form_params[0], $form_params[1], $field);

    return;
}

/**
 * Form Submit. Add a form group.
 */
function so_forms_add_group_submit($form, &$form_state) {

    $group = array(
        'id' => '',
        'sfid' => $form_state['values']['sfid'],
        'label' => $form_state['values']['add_group']['label'],
        'field' => '',
        'params' => array(
            'form_region' => 'advanced',
            'enabled' => false
        ),
        'field_type' => 'group',
        'widget' => '',
        'weight' => 50,
    );

    so_forms_store_field($group);

    return;
}

/**
 * Menu callback : delete a field
 *
 * @param int $id
 */
function so_forms_delete_field($id) {

    db_query('DELETE FROM {so_forms_fields} WHERE id=%d', $id);

    drupal_set_message('Field has been deleted');
    drupal_goto($_GET['destination']);
}

/**
 * Menu callback : delete a group
 *
 * @param int $id
 */
function so_forms_delete_group($id) {

    db_query("DELETE FROM {so_forms_fields} WHERE id = %d", $id);
    db_query("UPDATE {so_forms_fields} SET gid='' WHERE gid=%d", $id);

    drupal_set_message('Group has been deleted');
    drupal_goto($_GET['destination']);
}

/**
 * Form Submit. Save fields overview form.
 */
function so_forms_fields_overview_submit($form, &$form_state) {

    // BUG : lorsqu'on change un fieldset de région, la région des champs inclus ne change pas.

    // on aplatit le tableau multi-dimensionnel - TODO : c'est lourd : optimiser
    $values = array();

    foreach($form_state['values']['fields'] as $key => $value) {
        if(preg_match('#^group_#', $key)) {
            $subvalues = array();
            foreach($value as $subkey => $subvalue) {
                if(preg_match('#^field_#', $subkey)) {
                    $subvalues[$subkey] = $subvalue;
                }
            }
            $values[$key] = $value;
            $values = array_merge($values, $subvalues);
        } else {
            $values[$key] = $value;
        }
    }

    foreach($values as $key => $form_field) {

        $stored_field = so_forms_get_stored_field($form_field['fid']);

        $params = array_merge($stored_field->params, array(
            'form_region' => $form_field['form_region'],
            'enabled' => $form_field['enabled'],
        ));

        $record = array(
            'id' => $form_field['fid'],
            'label' => $form_field['label'],
            'field_values' => $stored_field->field_values,
            'params' => $params,
            'weight' => $form_field['weight'],
            'gid' => $form_field['pfid'],
        );

        so_forms_store_field($record);
    }

    drupal_set_message(t("Form's configuration has been saved"));

    return;
}

/**
 * Theme the fields overview form
 */
function theme_so_forms_fields_overview($form) {

    $form_params = so_forms_load_form_params($form['sfid']['#value']);
    $use_advanced_region = $form_params->configuration['advanced'];

    $rows = array();

    if($use_advanced_region == true) {
        $advanced_form_region_row = array(
            'data' => array(
                array(
                    'data' => t('Advanced form'),
                    'colspan' => 3,
                    'class' => 'region',
                ),
                array(
                    'data' => "<input type='text' class='form-pfid' value=''/>",
                ),
                array(
                    'data' => "",
                    'colspan' => 2,
                    'class' => 'region',
                ),
            ),
            'id' => 'form_fields_region_advanced',
            'class' => 'form_fields_region tabledrag-root tabledrag-leaf',
        );
    }

    // on aplatit le tableau multi-dimensionnel - TODO : c'est lourd : optimiser
    $listing_form = array();

    foreach(element_children($form['fields']) as $key) {
        if(preg_match('#^group_#', $key)) {
            $listing_subform = array();
            foreach(element_children($form['fields'][$key]) as $subkey) {
                if(preg_match('#^field_#', $subkey)) {
                    $listing_subform[$subkey] = $form['fields'][$key][$subkey];
                    unset($form['fields'][$key][$subkey]);
                }
            }
            $listing_form[$key] = $form['fields'][$key];
            $listing_form = array_merge($listing_form, $listing_subform);
        } else {
            $listing_form[$key] = $form['fields'][$key];
        }

        unset($form['fields'][$key]);
    }

    foreach($listing_form as $key => &$element) {

        if($element['form_region']['#value'] == 'advanced' && $element['#depth'] == 0  && isset($advanced_form_region_row)) {
            $rows[] = $advanced_form_region_row;
            unset($advanced_form_region_row);
        }

        $operations = array();
        foreach(element_children($element['operations']) as $op) {
            $operations[] = drupal_render($element['operations'][$op]);
        }
        while (count($operations) < 2) {
            array_unshift($operations, '');
        }

        // Add special classes to be used for tabledrag.js.
        $element['pfid']['#attributes']['class'] = 'form-pfid';
        $element['fid']['#attributes']['class'] = 'form-fid';
        $element['weight']['#attributes']['class'] = 'form-weight';
        $element['form_region']['#attributes']['class'] = 'form_region_text';

        $row = array();

        $row[] = theme('indentation', $element['#depth']) . drupal_render($element['label']);
        $row[] = drupal_render($element['field_infos']);
        $row[] = drupal_render($element['enabled']);
        $row[] = drupal_render($element['weight']) . drupal_render($element['pfid']) . drupal_render($element['fid']) . drupal_render($element['hidden_name']) . drupal_render($element['form_region']);
        $row = array_merge($row, $operations);

        if($element['#is_group'] == 1) {
            $classes = 'group draggable tabledrag-root';
        } else {
            $classes = 'group draggable tabledrag-leaf';
        }
        $rows[] = array('data' => $row, 'class' => $classes, 'id' => $key);
    }

    if (!empty($rows)) {
        if(isset($advanced_form_region_row)) {
            $rows[] = $advanced_form_region_row;
        }

        array_unshift($rows, array(
            'data' => array(
                array(
                    'data' => t("Normal form"),
                    'colspan' => 3,
                    'class' => 'region',
                ),
                array(
                    'data' => "<input type='text' class='form-pfid' value=''/>",
                ),
                array(
                    'data' => "",
                    'colspan' => 2,
                    'class' => 'region',
                ),
            ),
            'id' => 'form_fields_region_normal',
            'class' => 'form_fields_region tabledrag-root tabledrag-leaf',
            'style' => ($use_advanced_region == true ? "" : "display: none;"),
        ));

        drupal_add_tabledrag('so_form_fields', 'match', 'parent', 'form-pfid', null, 'form-fid', false, 1);
        drupal_add_tabledrag('so_form_fields', 'order', 'sibling', 'form-weight', null, null, true);

        drupal_add_js("
Drupal.behaviors.soFormsDrag = function(context) {

    var formDrag = Drupal.tableDrag.so_form_fields;

    formDrag.onDrop = function() {
        var draggedElement = $(this.rowObject.element);
        var formRegion = draggedElement.prevAll('tr.form_fields_region').attr('id').replace('form_fields_region_', '');
        draggedElement.find('input.form_region_text').val(formRegion);
    }

};
        ", 'inline');

        // En-tête du tableau
        $header = array(
            t('Field'),
            t('Type'),
            t('Enabled'),
            t('Weight'),
            array('data' => t('Operations'), 'colspan' => '2'),
        );
    }

    $output = theme('table', $header, $rows, array('id' => 'so_form_fields'));

    $output .= drupal_render($form['informations']);

    $output .= drupal_render($form['submit']);

    if($form_params->configuration['api_only'] != true) {
        $rows = array();
        // Ajout du formulaire de création de champ
        $row = array();
        $element = &$form['add_field'];
        $row[] = drupal_render($element['label']);
        $row[] = drupal_render($element['field']);
        $row[] = drupal_render($element['field_wrapper']);
        $row[] = array('data' => drupal_render($element['submit']), 'style' => 'text-align: right;');
        $rows[] = array('data' => $row);

        $header = array(
                array('data' => t('Add field'), 'colspan' => '4')
        );

        $output .= theme('table', $header, $rows, array('id' => 'add_field'));
    }

    if($form_params->configuration['no_groups'] != true) {
        $rows = array();
        // Ajout du formulaire de création de groupe
        $row = array();
        $element = &$form['add_group'];
        $row[] = drupal_render($element['label']);
        $row[] = array('data' => drupal_render($element['submit']), 'style' => 'text-align: right;');
        $rows[] = array('data' => $row);

        $header = array(
                array('data' => t('Add group'), 'colspan' => '2')
        );

        $output .= theme('table', $header, $rows, array('id' => 'add_group'));
    }

    return $output . drupal_render($form);
}

/**
 * Form field's configuration form
 *
 * @param array $form_state
 * @param int $id database id
 * @param string $lang
 *
 * @return array
 */
function so_forms_edit_field($form_state, $id, $lang = null) {

    $form = array();

    if($lang == null) {
        $lang = language_default('language');
    }

    $field_instance = so_forms_get_field_instance($id, $lang);

    drupal_set_title(t("Field '%field' configuration", array('%field' => $field_instance->getFieldLabel())));

    $form = $field_instance->fieldConfigurationForm($form_state);

    $form['#sf_id'] = $id;
    $form['#sf_language'] = $lang;

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t("Save"),
    );

    $form['back'] = array(
        '#value' => l(t("Back"), $_GET['destination']),
    );

    return $form;
}

/**
 * Form field's configuration form validate
 */
function so_forms_edit_field_validate(&$form, &$form_state) {

    $field_instance = so_forms_get_field_instance($form['#sf_id'], $form['#sf_language']);
    $field_instance->fieldConfigurationFormValidate($form, $form_state);
}

/**
 * Form field's configuration form submit
 */
function so_forms_edit_field_submit($form, &$form_state) {

    $field_instance = so_forms_get_field_instance($form['#sf_id'], $form['#sf_language']);
    $field_instance->fieldConfigurationFormSubmit($form, $form_state);

    $field = (array)so_forms_get_stored_field($form['#sf_id']);
    $field['params']['configuration'] = $field_instance->getFieldConfiguration();

    // le champ a demandé a être désactivé (par exemple : absence de valeurs, ou paramètre encore vierge, etc.)
    if($field_instance->fieldShouldBeDisabled() == true) {
        $field['params']['enabled'] = false;
        drupal_set_message("Field has been disabled. You'll have to enable it again manually.", 'warning');
    }

    $field['field_values'] = $field_instance->getFieldValues();

    so_forms_store_field($field);

    $form_state['rebuild'] = true;

    drupal_set_message("Field's configuration has been saved");
}

/**
 * Implementation of hook_theme()
 * @param array $form
 */
function theme_so_forms_edit_field($form) {
    $output = "";

    $field_instance = so_forms_get_field_instance($form['#sf_id'], $form['#sf_language']);
    $output .= $field_instance->themeFieldConfigurationForm($form);

    // form token, form_id, etc. au cas où ça ne serait pas déjà effectué dans une des deux classes.
    $output .= drupal_render($form);

    return $output;
}

/**
 * AHAH : populates field_type select box
 */
function so_forms_js_field_type_options($sfid) {

    $form_build_id = $_POST['form_build_id'];
    $field = $_POST['add_field']['field'];

    if(!empty($field)) {

        $fields_pool = so_forms_load_fields_pool($sfid);
        $field_types = (array)$fields_pool[$field]['type'];
        $registered_types = so_forms_registered_fields_types();
        $registered_widgets = so_forms_registered_widgets();

        $options = array();

        foreach($field_types as $field_type) {

            if(!empty($registered_types[$field_type])) {
                $available_widgets = $registered_types[$field_type]['class']::getCompatibleWidgets($fields_pool[$field]);

                if(!empty($fields_pool[$field]['widget'])) {
                    $available_widgets = array_intersect((array)$fields_pool[$field]['widget'], $available_widgets);
                }

                $options[t($registered_types[$field_type]['label'])] = array();

                foreach($available_widgets as $widget) {
                    $options[t($registered_types[$field_type]['label'])][$field_type . ':' . $widget] = t($registered_widgets[$widget]['label']);
                }

            } else {
                $error_msg = "<span class='marker'>" . t("Unregistered field type found") . "</span>";
            }
        }

        if(!empty($options)) {
            $element = array(
                '#type' => 'select',
                '#title' => t("Field type"),
                '#description' => $error_msg,
                '#options' => $options,
                '#id' =>'field_type',
                '#attributes' => array('style' => 'max-width: 200px;'),
                '#prefix' => "<div id='field_type_wrapper'>",
                '#suffix' => "</div>",
                '#attributes' => array('style' => 'max-width: 200px;'),
            );

            global $user;

            if($user->uid == 1 && !empty($fields_pool[$field]['callbacks']['values'])) {

                global $language;

                $function = $fields_pool[$field]['callbacks']['values'];

                $field_infos = explode(':', $field);

                $values = $function(array(
                    'field_name' => $field_infos[1],
                    'field_context' => $field_infos[2],
                    'field_module' => $field_infos[0],
                    'field_type' => $fields_pool[$field]['type'],
                ), $language->language, null);

                if(!empty($values)) {

                    foreach($values as $value) {
                        $list .= (is_array($value) ? $value['alias'] : check_plain($value)) . "<br />";
                    }

                    $field_values = array(
                        '#type' => 'fieldset',
                        '#title' => t("Field values"),
                        '#collapsible' => true,
                        '#collapsed' => true,
                        'values' => array(
                            '#type' => 'markup',
                            '#prefix' => "<div>",
                            '#suffix' => "</div>",
                            '#value' => $list,
                        ),
                    );
                } else {
                    $field_values = array(
                        '#type' => 'markup',
                        '#value' =>  t("No values"),
                        '#prefix' => "<div>",
                        '#suffix' => "</div>",
                    );
                }
            }
        } else {
            return drupal_json($error_msg);
        }
    } else {
        $element = array(
            '#type' => 'item',
            '#title' => t("Field type"),
            '#value' => t("Select a field"),
            '#prefix' => "<div id='field_type_wrapper'>",
            '#suffix' => "</div>",
            '#attributes' => array('style' => 'max-width: 200px;'),
        );
    }

    // on initialise un form_state
    $form_state = array('storage' => NULL, 'submitted' => FALSE);

    // on extrait la structure de base du formulaire du cache (état non submited)
    $form = form_get_cache($form_build_id, $form_state);

    $form['add_field']['field_wrapper']['field_type'] = $element;
    $form['add_field']['field_wrapper']['field_values'] = $field_values;

    // on sauvegarde la structure du formulaire modifié dans le cache
    form_set_cache($form_build_id, $form, $form_state);

    // on construit le formulaire (données POST mappées)
    $form = form_builder($form_id, $form, $form_state);

    // on effectue le rendu des éléments
    $output = drupal_render($form['add_field']['field_wrapper']['field_type']);
    $output .= drupal_render($form['add_field']['field_wrapper']['field_values']);

    return drupal_json($output);
}

/**
 * Callback for AHAH/AJAX operations.
 *
 * @param string $secured_id
 * @param string $op
 * @param ... additionals arguments to pass to the class method
 *
 * @return string
 */
function so_forms_js_callback($secured_id, $op) {
    $args = (array)func_get_args();
    $args = array_slice($args, 1);

    $field_instance = so_forms_get_field_instance($secured_id);

    $output = call_user_func_array(array($field_instance, 'jsCallback'), $args);

    return drupal_json($output);
}