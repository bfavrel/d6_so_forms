<?php
/**
 * Variables : 
 *      - $form_name : human readable form name/title
 *      - $secured_sfid : the sfid to use for any AJAX/AHAH operations or in any other situations where the id of the form has to be exposed.
 *          
 *      - $normal_fields : fields to show at start
 *      - $advanced_fields : additional fields. Used with empty() helps to determine if an 'advanced' button is needed. 
 *      - $submit : submit button
 *      - $reset : reset button : button element is disabled if form isn't populated
 *
 *      - $form_is_populated : (boolean) indicates whether or not the form has been populated with memorized user inputs.
 *          Useful to add a reminder to the user like an outline around the form, an exclamation mark icon, etc.
 *          This value can be also used to make the 'reset' button grayed, since it's disabled when no value is available.
 *  
 *      - $advanced_form_is_populated : (boolean) indicates whether or not advanced region of the form contains some user values.
 *          Useful to determine whether this region should be visible or not at start. 
 * 
 *      - $form_params : internal stuff required by form processing (form_id, token, etc.) 
 * 
 * @see so_forms/theme/theme.inc:so_forms_preprocess_so_form()
 */
?>

<?php if(!empty($normal_fields) || !empty($advanced_fields)): ?>

    <div class="normal_form">
        <?php print($normal_fields); ?>
    </div>

    <div class="advanced_form">
        <?php print($advanced_fields); ?>
    </div>

    <div class="form_controls">
        <?php print($reset); ?>
        <?php print($submit); ?>    
    </div>

    <?php print($form_params); ?>

<?php endif; ?>