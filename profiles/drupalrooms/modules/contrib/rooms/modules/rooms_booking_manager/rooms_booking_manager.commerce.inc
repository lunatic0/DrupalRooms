<?php

/**
 * @file
 * Contains the main integration points with Drupal Commerce - eventually all commerce-related functions should move here
 * here.
 */


function rooms_create_line_item($unit_to_book, AvailabilityAgent $agent) {
    $date_format = variable_get('rooms_date_format', 'd-m-Y');

    $unit_id = $unit_to_book['unit']->unit_id;
    $status = $unit_to_book['state'];

    $unit = rooms_unit_load($unit_id);

    $unit_type = rooms_unit_type_load($unit->type);

    // Set the correct departure date for the product name (checkout day)
    $dd = new DateTime($agent->end_date->format('Y-m-d'));
    $dd->add(new DateInterval('P1D'));

    $product_id = variable_get('rooms_booking_manager_booking_product_id', 0);
    $room_product = commerce_product_load($product_id);
    $line_item = commerce_product_line_item_new($room_product, 1, 0, array(), 'rooms_booking');

    // Product name - we set something that will make sense to the user here - this needs to be parametrised of course
    $nights = $dd->diff($agent->start_date)->d + 1;
    $line_item_label = 'Booking for ' . $unit_type->label . ' (' . $nights . ' Nights; Arrival: ' . $agent->start_date->format($date_format) . ' Departure: ' . $dd->format($date_format) . ')';
    $line_item->line_item_label = $line_item_label;

    $unit_info = $agent->checkAvailabilityForUnit($unit_id);

    // Check $unit_info to make sure we still have availbility
    if ($unit_info == ROOMS_NO_ROOMS) {
      form_set_error('', t('We are soory, booking unit is no longer available.'));
    }
    else {
      // If available set price and create line item
      $unit_first = array_pop($unit_info);
      $price = $unit_first['price'];

      // Convert to integer value as this is what Commerce expects
      $price = commerce_currency_decimal_to_amount($price, commerce_default_currency());

      // We do not need to setup the line item's commerce_unit_price here, as it will be overwritten
      // with the values from the product entity anyway. Setting commerce unit price
      // to the correct value is now done completely by the function
      // rooms_booking_manager_commerce_cart_line_item_refresh() in rooms_booking_manager.module

      $commerce_dates = array(
        LANGUAGE_NONE => array( array(
          'value' => $agent->start_date->format('Y-m-d'),
          'value2' => $agent->end_date->format('Y-m-d'),
        )),
      );

      $commerce_unit = array(
        LANGUAGE_NONE => array( array(
          'value' => $unit->unit_id,
        )),
      );

      $commerce_status = array(
        LANGUAGE_NONE => array(array(
          'value' => $status,
        )),
      );

      $commerce_price = array(
        LANGUAGE_NONE => array(array(
          'value' => $price,
        )),
      );

      $booking_price = array(
        LANGUAGE_NONE => array(array(
          'value' => $unit_first['booking_price'],
        )),
      );

      $group_size = array(
        LANGUAGE_NONE => array(
          array(
            'value' => $agent->group_size,
          ),
          array(
            'value' => $agent->group_size_children,
          ),
        ),
      );

      $line_item->rooms_booking_number_people = $group_size;
      $line_item->rooms_booking_dates = $commerce_dates;
      $line_item->rooms_booked_unit_id = $commerce_unit;
      $line_item->rooms_booked_status = $commerce_status;
      $line_item->rooms_booked_price = $commerce_price;
      $line_item->rooms_booked_bookingprice = $booking_price;

      return $line_item;
    }
}
