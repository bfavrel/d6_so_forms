<?php

class FormWidgetTextfield extends FormWidgetAbstract
{
    public function render(array $configuration, array $stored_values, array $default_value) {
        $element = array(
            '#type' => 'textfield',
            '#default_value' => $default_value[0], // attention, les entrées utilisateur brutes sont des tableaux
        );

        return $element;
    }
    
    public function compileValues(array $raw_value, array $configuration, array $stored_values) {
        return $raw_value;
    }
}
