# access_code_encryptor.py
#
#   James Butcher
#   7/20/22
#
#   1. Reads the raw, plain-text access codes (passwords) from "access_codes_factory.txt" and "access_codes_service.txt"
#   2. Encrypts the codes using the cryptofy method
#   3. Writes the encrypted codes to output folder files: "fc.txt" and "sc.txt", where they are staged to be synced with GitHub


from CommonUtilities.Security.cryptofy_functions import cryptofy


# Read in raw access codes

with open('LaserController/access_codes_factory.txt', 'r') as factory_codes_file:
    factory_codes = []
    for line in factory_codes_file.readlines():
        factory_codes.append(line.strip())

with open('LaserController/access_codes_service.txt', 'r') as service_codes_file:
    service_codes = []
    for line in service_codes_file.readlines():
        service_codes.append(line.strip())

print("Factory passwords:")
print(factory_codes)
print()

print("Service passwords:")
print(service_codes)
print()

# Write encrypted access codes to file to be uploaded to GitHub

with open('Output/fc.txt', 'w') as encrypted_factory_codes_file:
    for code in factory_codes:
        encrypted_factory_codes_file.write(cryptofy(code) + "\n")

with open('Output/sc.txt', 'w') as encrypted_service_codes_file:
    for code in service_codes:
        encrypted_service_codes_file.write(cryptofy(code) + "\n")
