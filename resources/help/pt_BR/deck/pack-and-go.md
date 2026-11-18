# Pack and Go: leve uma apresentação com você
<!-- keywords: pack and go, empacotar, enviar, compartilhar, outro computador, zip, pasta, fontes, arquivos vinculados, imagens vinculadas, imagens ausentes, copiar, levar junto, pen drive, licença, filme, som, vídeo, áudio, apresentação -->

Uma apresentação pode reproduzir um filme ou um som de um arquivo que não está dentro dela. Em outro computador, esse arquivo não existe, e a apresentação fica com uma falha. O **Pack and Go** salva a apresentação junto com seus arquivos vinculados e suas fontes, em um único arquivo ZIP ou em uma única pasta.

## Como usar

1. Clique em **Arquivo** e depois em **Exportar**.
2. Clique em **Pack and Go: Salvar para Outro Computador** para criar um arquivo ZIP, ou em **Pack and Go: Salvar em uma Pasta** para criar uma nova pasta.
3. Escolha onde salvar. Para um arquivo ZIP, digite um nome. Para uma pasta, escolha a pasta em que a nova pasta será criada.
4. Leia a mensagem que aparece. Ela informa onde está o pacote, quantos arquivos vinculados e arquivos de fonte foram incluídos e tudo o que não pôde ser empacotado.

Um **arquivo ZIP** é um único arquivo que contém outros arquivos, por isso é fácil de enviar. A outra pessoa o abre para extrair os arquivos. Uma pasta é melhor quando você quer copiar tudo para um pen drive.

## O que vai para o pacote

- **A própria apresentação**, com o mesmo nome que tem agora. É uma cópia. Seu arquivo original não muda.
- **Filmes e sons vinculados.** Um filme ou um som pode ser vinculado, de modo que a apresentação o reproduz a partir do seu arquivo, em vez de guardá-lo. O pacote inclui uma cópia de cada um, para que a apresentação funcione da mesma forma no outro computador. Se você não inseriu o arquivo na apresentação, é assim que você o leva junto.
- **Imagens vinculadas.** O pacote inclui uma cópia de cada imagem vinculada ao seu arquivo.
- **Fontes.** Uma fonte é o visual das letras. O pacote inclui os arquivos de fonte que acompanham o programa e que a apresentação usa, em uma pasta chamada **fonts**. O Pack and Go nunca empacota uma fonte cuja licença proíba sua distribuição. A mensagem informa o nome de cada fonte que ficou de fora. As fontes instaladas no seu computador por você não são empacotadas, por isso a outra pessoa talvez precise delas.

## Como os vínculos mudam

Na cópia, cada vínculo que foi empacotado aponta para o arquivo dentro do pacote, por meio de um nome curto, como **linked/photo.png**. Os vínculos não usam mais o caminho do seu computador, por isso funcionam em qualquer lugar onde o pacote for aberto. Se dois arquivos tiverem o mesmo nome, ao segundo é acrescentado um número.

Um vínculo que o Pack and Go não consegue seguir permanece como estava, e a mensagem o lista. O Pack and Go não empacota:

- Um arquivo que não pode ser encontrado.
- Um endereço da web. Esses permanecem como estão.
- Um arquivo em outro computador da sua rede.
- Um vínculo que sobe e sai da pasta da apresentação, como **../../notas.png**.
- Um documento, uma pasta de trabalho ou outro objeto que não esteja na própria pasta da apresentação (ou em uma pasta dentro dela), mesmo quando o vínculo informa o caminho completo. Somente imagens, vídeos e sons são empacotados de qualquer lugar.
- Um vínculo que sai da pasta da apresentação por meio de um atalho para outra pasta (um link simbólico).
- Um vínculo para a própria apresentação.
- Um arquivo que não seja imagem, vídeo, som ou documento. Um programa nunca é empacotado.

O Pack and Go não procura vínculos dentro dos arquivos vinculados, por isso um arquivo que aponte de volta para esta apresentação não pode causar um ciclo.

## Limites

Para que o pacote não fique grande demais, o Pack and Go pergunta antes quando há mais de 200 arquivos vinculados ou mais de 256 MB deles. Escolha **Não** e nada será salvo. O Pack and Go nunca empacota mais de 2.000 arquivos, nem mais de 1 GB, qualquer que seja a sua resposta. Os arquivos além desse limite ficam de fora e são listados na mensagem.

Se você usar uma senha na apresentação, a cópia no pacote terá a mesma senha.
