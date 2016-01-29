/*! \file
 *  \brief File containing the declaration of the \ref device_manager class.
 *  
 *  File containing the header information and declaration of the
 *  \ref device_manager class. This file includes the minimal number of files
 *  necessary to use any instance of the \ref device_manager class.
 *  
 *  \author Daniel Andrus
 *  \date 2015-09-08
 *  \copyright Copyright (c) 2015 7400 Circuits. All rights reserved.
 */

#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <mutex>
#include <thread>
#include <vector>

namespace usb {
class usb_device;
}
class linkmasta_device;



/*!
 *  \brief Manages connected \ref linkmasta_device devices.
 *  
 *  Manages connected \ref linkmasta_device devices, providing methods for
 *  listing connected devices, getting metadata from the devices, and claiming
 *  devices as a way of prevening concurrent modification in multi-threaded
 *  environments. Also provides mechanisms for backgroud polling to determine
 *  when devices have become connected or disconnected.
 *  
 *  Any implementations of this class should be designed to be thread-safe.
 */
class device_manager
{
public:
  
  /*!
   *  \brief Class constructor. Initializes member variables to default values.
   *  
   *  Class constructor. Initializes member variables to default values.
   */
                                    device_manager();
  
  /*!
   *  \brief Class destructor. Deallocates dynamically allocated memory and
   *         joins with any detached threads.
   *  
   *  Class destructor. Deallocates dynamically allocated memory and joins with
   *  any detached threads.
   */
  virtual                           ~device_manager();
  
  
  
  /*!
   *  \brief Gets a list of IDs of connected \ref linkmasta_device devices.
   *  
   *  Gets a list of IDs of connected \ref linkmasta_device devices.
   *  
   *  \return A \ref std::vector of IDs that can be used to get information on
   *          connected \ref linkmsata_device devices.
   */
  virtual std::vector<unsigned int> get_connected_devices() = 0;
  
  /*!
   *  \brief Attempts to get the list of connected devices if the manager is not
   *         busy.
   *  
   *  Attempts to get the list of connected \ref linkmasta_device devices with
   *  the option to fail. This method will not be blocked if another thread is
   *  currently using this class, but instead will return false.
   *  
   *  \param [out] devices A \ref std::vector of IDs for connected
   *         \ref linkmasta_device devices to write the results of this function
   *         call. If this function fails for whatever reason, than this
   *         parameter will remain unmodified.
   *  
   *  \return true if the provided parameter was modified to contain the device
   *          IDs of all connected \ref linkmasta_device devices, false if the
   *          operation failed to do that.
   */
  virtual bool                      try_get_connected_devices(std::vector<unsigned int>& devices) = 0;
  
  /*!
   *  \brief Checks if a \ref linkmasta_device with the given ID is still
   *         connected to the system.
   *  
   *  Checks if a \ref linkmastas_device with the given ID is still connected to
   *  the system. Will return a boolean value indicating whether the device is
   *  connected or not.
   *  
   *  \param [in] id The ID number of the device to check. Must be a valid ID
   *         number of a connected \ref linkmasta_device.
   *  
   *  \return true if the device is still connected, false if it has been
   *          disconnected.
   */
  virtual bool                      is_connected(unsigned int id) = 0;
  
  /*!
   *  \brief Gets the vendor ID of the \ref linkmasta_device with the given ID.
   *  
   *  Gets the vendor ID of the \ref linkmasta_device with the given ID. If no
   *  \ref linkmasta_device with the provided ID exists, then an exception will
   *  be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the information for.
   *         Must be a valid ID number of a connected \ref linkmasta_device.
   *  
   *  \return The vendor ID of the connected \ref linkmasta_device matching the
   *          provided ID.
   */
  virtual unsigned int              get_vendor_id(unsigned int id) = 0;
  
  /*!
   *  \brief Gets the product ID of the \ref linkmasta_device with the given ID.
   *  
   *  Gets the product ID of the \ref linkmasta_device with the given ID. If no
   *  \ref linkmasta_device with the provided ID exists, then an exception will
   *  be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the information for.
   *         Must be a valid ID number of a connected \ref linkmasta_device.
   *  
   *  \return The product ID of the connected \ref linkmasta_device matching the
   *          provided ID.
   */
  virtual unsigned int              get_product_id(unsigned int id) = 0;
  
  /*!
   *  \brief Gets the manufacturer string of the \ref linkmasta_device with the
   *         given ID.
   *  
   *  Gets the manufacturer string of the \ref linkamsta_device with the given
   *  ID. If no \ref linkmasta_device with the provided ID exists, then an
   *  exception will be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the information for.
   *         Must be a valid ID number of a connected \ref linkmasta_device.
   *  
   *  \return The manufacturer string of the connected \ref linkamsta_device
   *          matching the provided ID.
   */
  virtual std::string               get_manufacturer_string(unsigned int id) = 0;
  
  /*!
   *  \brief Gets the product string of the \ref linkmasta_device with the given
   *         ID.
   *  
   *  Gets the product string of the \ref linkamsta_device with the given ID. If
   *  no \ref linkmasta_device with the provided ID exists, then an exception
   *  will be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the information for.
   *         Must be a valid ID number of a connected \ref linkmasta_device.
   *  
   *  \return The product string of the connected \ref linkamsta_device matching
   *          the provided ID.
   */
  virtual std::string               get_product_string(unsigned int id) = 0;
  
  /*!
   *  \brief Gets the serial number string of the \ref linkamsta_device with the
   *         given ID.
   *  
   *  Gets the serial number string of the \ref lnkamsta_device with the given
   *  ID. If no \ref linkmasta_device with the provided ID exists, then an
   *  exception will be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the information for.
   *         Must be a valid ID number of a connected \ref linkamsta_device.
   *  
   *  \return The serial number string of the connected \ref linkmasta_device
   *          matching the provided ID.
   */
  virtual std::string               get_serial_number(unsigned int id) = 0;
  
  /*!
   *  \brief Gets a pointer to a \ref linkamsta_device object that can be used
   *         to interact with the device.
   *  
   *  Gets a pointer to a \ref linkmasta_device object that can be used to
   *  interact with and manipulate the device. If no \ref linkmasta_device with
   *  the provided ID exists, then an exception will be thrown.
   *  
   *  \param [in] id The ID number of the device to fetch the
   *         \ref linkmasta_device pointer for. Must be a valid ID number of a
   *         connected \ref linkmasta_device.
   *  
   *  \return A pointer to a \ref linkmasta_device class that can be used to
   *          interact with the device.
   */
  virtual linkmasta_device*         get_linkmasta_device(unsigned int id) = 0;
  
  /*!
   *  \brief Tests to see if the desired \ref linkmasta_device has been
   *         previously claimed.
   *  
   *  Tests to see if the desired \ref linkmasta_device has been previously
   *  claimed. If a device has been claimed, then it should not be accessed or
   *  modified in any way until it has been released. If no
   *  \ref linkmasta_device with the provided ID exists, then an exception will
   *  be thrown.
   *  
   *  A device can be claimed with a call to \ref try_claim_device(unsigned int)
   *  and can be released with a call to \ref release_device(unsigned int).
   *  
   *  \param [in] id The ID number of the device to check. Must be a valid ID
   *         number of a connected \ref linkmasta_device.
   *  
   *  \return true if the device is currently claimed, false if not.
   */
  virtual bool                      is_device_claimed(unsigned int id) = 0;
  
  /*!
   *  \brief Attempts to claim the desired \ref linkmasta_device that has the
   *         provided ID.
   *  
   *  Attempts to claim the desired \ref linkamsta_device that has the provided
   *  ID. If the device is already claimed, then function will *not* claim the
   *  device again and will return false. If this function is successful, then
   *  it will return true, indicating that it is safe to manipulate the device.
   *  If no \ref linkmasta_device with the provided ID exists, then an exception
   *  will be thrown.
   *  
   *  \param [in] id The ID number of the device to attempt to claim. Must be a
   *         valid ID number of a connected \ref linkmasta_device.
   *  
   *  \return true if this function successfully claimed the device, false if
   *          not and the device is already claimed.
   */
  virtual bool                      try_claim_device(unsigned int id) = 0;
  
  /*!
   *  \brief Releases the claim on the \ref linkmasta_device that has the
   *         provided ID.
   *  
   *  Releases the claim on the \ref linkamsta_device that has the provided ID.
   *  This should only be called by the same one that originally claimed the
   *  device.  If no \ref linkmasta_device with the provided ID exists, then an
   *  exception will be thrown.
   *  
   *  \param [in] id The ID number of the device to release any claims for. Must
   *         be a valid ID number of a connected \ref linkmasta_device.
   */
  virtual void                      release_device(unsigned int id) = 0;
  
  
  
protected:
  
  /*!
   *  \brief Generates a new unique device ID that can be used as an identifier
   *         for a newly connected \ref linkmasta_device device.
   *  
   *  Generates a new unique device ID that can be used as an identifier for a
   *  newly conencted \ref linkmasta_device device. Subsequent calls to this
   *  function will never produce the same result, and thus the results of each
   *  call should be immediately stored.
   *  
   *  \return A new unique device ID for a new \ref linkamsta_device device.
   */
  unsigned int                      generate_id();
  
  /*!
   *  \brief Initializes an auto-refreshing background thread that will
   *         automatically call the \ref refresh_device_list() function on a
   *         periodic interval.
   *  
   *  Initializes a repeating background thread that will automatically call the
   *  \ref refresh_device_list() function on a periodic interval. If another
   *  auto-refresh thread is already running, then this function will do
   *  nothing. This new thread can be stopped by calling
   *  \ref stop_auto_refresh_and_wait().
   *  
   *  After a predesignated time, this background thread will execute the
   *  \ref refresh_device_list() function.
   */
  void                              start_auto_refresh();
  
  /*!
   *  \brief Sends a stopping signal to the auto-refresh thread and blocks the
   *         current thread until the background thread completes and joins the
   *         current thread.
   *  
   *  Sends a stopping signal to the auto-refresh thread and blocks the current
   *  current thread until the background thread completes and joins the current
   *  thread. If no background thread is running, then this function will do
   *  nothing.
   *  
   *  The auto-refresh thread can be started with a call to
   *  \ref start_auto_refresh().
   */
  void                              stop_auto_refresh_and_wait();
  
  /*!
   *  \brief Polls connected devices to test for new connections or disconnected
   *         devices and updates any member variables to track these changes.
   *  
   *  Poll connected devices to test for new connections or disconnected devices
   *  and upates any member variables to track these changes.
   */
  virtual void                      refresh_device_list() = 0;
  
  /*!
   *  \brief Constructs a \ref linkmasta_device object using the provided
   *         \ref usb::usb_device object as a handle.
   *  
   *  Constructs a \ref linkmasta_device object using the provided
   *  \ref usb::usb_device object as a handle.
   *  
   *  \param [in|out] The \ref usb::usb_device object to use when constructing
   *         the new \ref linkmasta_device device.
   *  
   *  \return The new \ref linkmasta_device.
   */
  static linkmasta_device*          build_linkmasta_device(usb::usb_device* device);
  
  
  
private:
  
  /*!
   *  \brief Disallows implicit or explicit copies of this class.
   */
                                    device_manager(const device_manager& other) = delete;
  
  /*!
   *  \brief The function that the auto-refresh thread executes. Includes
   *         mechanisms for periodic execution of \ref refresh_device_list() and
   *         safely completes when the \ref m_thread_kill_flag variable is
   *         set.
   */
  void                              refresh_thread_function();
  
  
  
private:
  
  /*! \brief Handle for the periodic device refreshing thread. */
  std::thread                       m_refresh_thread;
  
  /*! \brief Flag telling the device refreshing thread to complete. */
  bool                              m_thread_kill_flag;
  
  /*! \brief Flag indicating that the device refreshing thread has completed. */
  bool                              m_thread_dead;
  
  /*!
   *  \brief The current device id to return on the next call to
   *         \ref generate_id().
   */
  unsigned int                      curr_id;
};

#endif /* defined(__DEVICE_MANAGER_H__) */
