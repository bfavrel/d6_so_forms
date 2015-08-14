<?php

class FormFieldNumerical extends FormFieldAbstract
{
    public static function getCompatibleWidgets(array $field_definition) {

        $implemented_widgets = array(
            0 => 'textfield',
        );

        return $implemented_widgets;
    }

    public function fieldConfigurationForm(array $form_state) {

        // ce widget possède t-il un formulaire de configuration ?
        if(empty($form_state)) {
            return true;
        }

        $form = array();

        $form['#tree'] = true;
        $form['#submit'] = array('so_forms_edit_field_submit'); // obligatoire afin d'avoir les submit callback dans le bon ordre

        $form['configuration']['module_custom'] = $this->executeCallback($this->_configuration_callback);

        switch($this->_widget_name) {

            case 'textfield':
                $form['configuration']['prefix'] = array(
                    '#type' => 'textfield',
                    '#title' => t("Field prefix"),
                    '#default_value' => $this->_configuration['prefix'],
                    '#size' => 20,
                    '#weight' => 0,
                );

                $form['configuration']['suffix'] = array(
                    '#type' => 'textfield',
                    '#title' => t("Field suffix"),
                    '#default_value' => $this->_configuration['suffix'],
                    '#size' => 20,
                    '#weight' => 1,
                );
                
                $form['configuration']['display_label'] = array(
                    '#type' => 'checkbox',
                    '#title' => t("Display the field label"),
                    '#default_value' => $this->_configuration['display_label'] === null ? 1 : $this->_configuration['display_label'],
                    '#weight' => 2,
                );

                break;
        }

        $form = array_merge_recursive($form, (array)$this->_widget->widgetConfigurationForm($form_state, $this->_configuration, $this->_language, $this->_stored_values, null));

        return $form;
    }

    public function fieldConfigurationFormValidate(array &$form, array &$form_state) {
        $this->_widget->widgetConfigurationFormValidate($form, $form_state);
    }

    public function fieldConfigurationFormSubmit(array $form, array &$form_state) {

        $this->_configuration = $form_state['values']['configuration'];

        $this->_widget->widgetConfigurationFormSubmit($form, $form_state, $this->_configuration, $this->_stored_values);

        if(!empty($form_state['values']['configuration']['module_custom'])) {
            $this->_configuration['module_custom'] = $form_state['values']['configuration']['module_custom'];
        }
    }

    public function render(array $default_value) {
        $element = array(
            '#type' => 'textfield',
            '#title' => $this->_label,
            '#default_value' => $default_value[0], // attention, les entrées utilisateur brutes sont des tableaux
            '#field_prefix' => $this->_configuration['prefix'],
            '#field_suffix' => $this->_configuration['suffix'],
            '#weight' => $this->_weight,
            '#attributes' => array('class' => 'form-numerical'),
        );
        
        if($this->_configuration['display_label'] === 0) {
            $element['#title'] = '';
        }

        if(!empty($this->_render_callback)) {
            // à cause de func_get_args() (copie des argument), il n'est malheureusement pas possible d'altérer l'élément par référence.
            $element = $this->executeCallback($this->_render_callback, $element);
        }

        return $element;
    }

    public function compileUserInputs(array $raw_value) {

        if($this->_widget_name == 'textfield' && is_numeric($raw_value[0]) == false) {return;}

        $user_input = parent::compileUserInputs($raw_value);
        $user_input = array_merge($user_input, array('values' => $this->_widget->compileValues($raw_value, $this->_configuration, $this->_stored_values)));

        return $user_input;
    }
}
