using System;
using Org.BouncyCastle.Crypto.Parameters;
using Org.BouncyCastle.Crypto.Engines;
using Org.BouncyCastle.Crypto.Modes;

namespace FEFileExplorer
{
    internal class Decryptor
    {
        private static byte[] key = System.Text.Encoding.UTF8.GetBytes("ifyouseethisyouhavetosubscribeee");

        public byte[] Decrypt(byte[] encrypted_output)
        {
            byte[] nonce = new byte[12];
            Array.Copy(encrypted_output, 0, nonce, 0, 12);

            int ciphertext_len = encrypted_output.Length - 12;
            byte[] ciphertext_with_tag = new byte[ciphertext_len];
            Array.Copy(encrypted_output, 12, ciphertext_with_tag, 0, ciphertext_len);

            GcmBlockCipher gcm = new GcmBlockCipher(new AesEngine());
            AeadParameters parameters = new AeadParameters(new KeyParameter(key), 128, nonce, null);
            gcm.Init(false, parameters);

            byte[] plaintext = new byte[gcm.GetOutputSize(ciphertext_with_tag.Length)];
            int len = gcm.ProcessBytes(ciphertext_with_tag, 0, ciphertext_with_tag.Length, plaintext, 0);
            len += gcm.DoFinal(plaintext, len);

            byte[] finalPlaintext = new byte[len];
            Array.Copy(plaintext, 0, finalPlaintext, 0, len);

            return finalPlaintext;
        }

    }
}
