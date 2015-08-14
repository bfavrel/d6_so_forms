<?php

class FormFieldText extends FormFieldAbstract
{

    public static function getCompatibleWidgets(array $field_definition) {
        $implemented_widgets = array(
            0 => 'textfield',
            1 => 'checkboxes',
            2 => 'select',
            3 => 'onoff',
            4 => 'radios',
        );

        // on ne propose pas de widgets de type 'choix de valeur' s'il n'y a pas moyen de récupérer la moindre valeur.
        if(empty($field_definition['callbacks']['values'])) {
            unset($implemented_widgets[1]); // checkboxes
            unset($implemented_widgets[2]); // select
            unset($implemented_widgets[3]); // onoff
            unset($implemented_widgets[4]); // radios
        }

        return $implemented_widgets;
    }

    public function fieldConfigurationForm(array $form_state) {

        // ce widget possède t-il un formulaire de configuration ?
        if(empty($form_state)) {
            $config_flag = false;

            // cela dépend du type de champ
            $config_flag |= $this->_widget_name == 'checkboxes' ||
                            $this->_widget_name == 'select' ||
                            $this->_widget_name == 'radios';
            // du widget
            $config_flag |= $this->_widget->widgetConfigurationForm($form_state, $this->_configuration, $this->_language, $this->_stored_values, $available_values);
            // et du module
            $config_flag |= $this->executeCallback($this->_configuration_callback);

            return (int)$config_flag;
        }

        $form = array();

        $form['#tree'] = true;
        $form['#submit'] = array('so_forms_edit_field_submit'); // obligatoire afin d'avoir les submit callback dans le bon ordre

        $form['configuration']['module_custom'] = $this->executeCallback($this->_configuration_callback);

        // si le module souhaite intervenir sur la soumission de ses propres éléments de form
        // TODO : dangereux : le module a accès à tous les paramètres : c'est la porte ouverte au bricolage. A corriger sous D7.
        array_unshift($form['#submit'], $form['configuration']['module_custom']['#submit']);
        unset($form['configuration']['module_custom']['#submit']);

        switch($this->_widget_name) {
            case 'checkboxes':
            case 'select':
            case 'radios':
                $form['configuration']['options_mode'] = array(
                    '#type' => 'radios',
                    '#title' => t("Type of options"),
                    '#default_value' => $this->_configuration['options_mode'],
                    '#options' => array(
                        1 => t("Use existing values as options"),
                        2 => t("Build custom options from existing values (unique use of values)"),
                        3 => t("Build custom options from existing values (multiple use of values)"),
                    ),
                    '#required' => true,
                    '#description' => t("Values will be updated after save"),
                    '#weight' => 0,
                );

                break;
        }

        $available_values = (array)$this->executeCallback($this->_values_callback);

        $form = array_merge_recursive($form, (array)$this->_widget->widgetConfigurationForm($form_state, $this->_configuration, $this->_language, $this->_stored_values, $available_values));

        return $form;
    }

    public function fieldConfigurationFormValidate(array &$form, array &$form_state) {
        $this->_widget->widgetConfigurationFormValidate($form, $form_state);
    }

    public function fieldConfigurationFormSubmit(array $form, array &$form_state) {

        if(!empty($form_state['values']['configuration']['module_custom'])) {
            $this->_configuration['module_custom'] = $form_state['values']['configuration']['module_custom'];
        }

        // on effectue d'abord la soumission du widget, afin d'éviter qu'il soumette des valeurs incohérentes résultant
        // d'un changement de mode des valeurs.
        $this->_disable = $this->_widget->widgetConfigurationFormSubmit($form, $form_state, $this->_configuration, $this->_stored_values);

        switch($this->_widget_name) {
            case 'checkboxes':
            case 'select':
            case 'radios':

                // si le mode des valeurs a changé le paramétrage des valeurs n'est plus d'actualité
                // on vide les paramètres de valeurs et on demande la désactivation du champ
                if($form_state['values']['configuration']['options_mode'] != $this->_configuration['options_mode']) {

                    if(!empty($this->_configuration['options_mode'])) {
                        $this->_disable = true;
                    }

                    $this->_configuration['options_mode'] = $form_state['values']['configuration']['options_mode'];
                    $this->_stored_values = array();

                    return;
                }

                break;
        }
    }

    public function render(array $default_value) {
        $element = array(
            '#title' => $this->_label,
            '#weight' => $this->_weight,
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
        $user_input = array_merge($user_input, array('values' => $this->_widget->compileValues($raw_value, $this->_configuration, $this->_stored_values)));

        return $user_input;
    }

}
