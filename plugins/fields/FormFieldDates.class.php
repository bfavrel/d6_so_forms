<?php

class FormFieldDates extends FormFieldAbstract
{
    public static function getCompatibleWidgets(array $field_definition) {
        $implemented_widgets = array(
            0 => 'datefields',
        );

        return $implemented_widgets;
    }

    public function fieldConfigurationForm(array $form_state) {

        // ce widget possède t-il un formulaire de configuration ?
        if(empty($form_state)) {
            $config_flag = false;

            // cela dépend :

            // du widget
            $config_flag |= $this->_widget->widgetConfigurationForm($form_state, $this->_configuration, $this->_language, $this->_stored_values, $available_values);
            // et du module
            $config_flag |= $this->executeCallback($this->_configuration_callback);

            return (int)$config_flag;
        }

        $form = array();

        $form['#tree'] = true;
        $form['#submit'] = array('so_forms_edit_field_submit'); // obligatoire afin d'avoir les submit callback dans le bon ordre
        
        // si le module souhaite intervenir sur la soumission de ses propres éléments de form
        // TODO : dangereux : le module a accès à tous les paramètres : c'est la porte ouverte au bricolage. A corriger sous D7.
        array_unshift($form['#submit'], $form['configuration']['module_custom']['#submit']);
        unset($form['configuration']['module_custom']['#submit']);
        
        $form['configuration']['use_field_label'] = array(
            '#type' => 'checkbox',
            '#title' => t("Use field label"),
            '#default_value' => isset($this->_configuration['use_field_label']) ? $this->_configuration['use_field_label'] : 0,
            '#weight' => 0,
        );

        $form['configuration']['module_custom'] = $this->executeCallback($this->_configuration_callback);

        $form = array_merge_recursive($form, (array)$this->_widget->widgetConfigurationForm($form_state, $this->_configuration, $this->_language, $this->_stored_values, $available_values));

        return $form;
    }

    public function fieldConfigurationFormValidate(array &$form, array &$form_state) {
        $this->_widget->widgetConfigurationFormValidate($form, $form_state);
    }

    public function fieldConfigurationFormSubmit(array $form, array &$form_state) {

        $this->_configuration['use_field_label'] = $form_state['values']['configuration']['use_field_label'];
        
        $this->_widget->widgetConfigurationFormSubmit($form, $form_state, $this->_configuration, $this->_stored_values);
        
        if(!empty($form_state['values']['configuration']['module_custom'])) {
            $this->_configuration['module_custom'] = $form_state['values']['configuration']['module_custom'];
        }
    }

    public function render(array $default_value) {
        $element = array(
            '#type' => 'fieldset',
            '#title' => $this->_configuration['use_field_label'] == 1 ? $this->_label : "",
            '#description' => $this->_configuration['description'],
            '#weight' => $this->_weight,
            '#attributes' => array('class' => 'datefields_wrapper'),            
        );

        $element = array_merge($element, (array)$this->_widget->render($this->_configuration, $this->_stored_values, $default_value));
        
        if(!empty($this->_render_callback)) {
            // à cause de func_get_args() (copie des argument), il n'est malheureusement pas possible d'altérer l'élément par référence.
            $element = $this->executeCallback($this->_render_callback, $element);
        } 

        return $element;
    }
    
    public function compileUserInputs(array $raw_value) {
        $user_input = parent::compileUserInputs($raw_value);
        
        $values = $this->_widget->compileValues($raw_value, $this->_configuration, $this->_stored_values);
        
        $from = DateTime::createFromFormat('Y-m-d H:i:s', $values['from']);
        $to = DateTime::createFromFormat('Y-m-d H:i:s', $values['to']);
        
        $user_input['values'] = array(
            'from' => $from != null ? $from->format('Y-m-d') : $values['from'],
            'to' => $to != null ? $to->format('Y-m-d') : $values['to'],
        );
        
        return $user_input;
    }
    
}


