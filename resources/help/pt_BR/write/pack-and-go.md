# Pack and Go: enviar um documento com o que ele precisa
<!-- keywords: pack and go, empacotar, pacote, enviar, compartilhar, outro computador, zip, pasta, fontes, vínculos, arquivos vinculados, imagens vinculadas, imagens ausentes, copiar, levar, pen drive, licença, INCLUDEPICTURE, INCLUDETEXT, campo -->

Quando você envia um documento para alguém, as imagens e as fontes que ele usa podem se perder no caminho. O **Pack and Go** salva o documento junto com o que ele precisa, em um arquivo ZIP ou em uma pasta, para que ele tenha a mesma aparência em outro computador.

## Como usar

1. Clique em **Arquivo** e depois em **Exportar**.
2. Clique em **Pack and Go: Salvar para Outro Computador** para criar um arquivo ZIP, ou em **Pack and Go: Salvar em uma Pasta** para criar uma nova pasta.
3. Escolha onde salvar. Para um arquivo ZIP, digite um nome. Para uma pasta, escolha a pasta em que a nova será criada.
4. Leia a mensagem que aparece. Ela informa onde está o pacote, quantos arquivos vinculados e arquivos de fonte foram incluídos e o que não pôde ser empacotado.

Um **arquivo ZIP** é um único arquivo que contém outros arquivos, por isso é fácil de enviar. A outra pessoa abre o arquivo para extrair os arquivos. Uma pasta é melhor quando você quer copiar tudo para um pen drive.

## O que vai para o pacote

- **O próprio documento**, com o mesmo nome que tem agora. É uma cópia. O seu arquivo não muda.
- **Imagens vinculadas.** Uma imagem vinculada mostra uma imagem de um arquivo, em vez de conter a própria imagem. O pacote guarda uma cópia de cada uma.
- **Arquivos citados por campos.** Um campo é um código que preenche o seu próprio resultado. Um campo **INCLUDEPICTURE** mostra uma imagem de um arquivo, e um campo **INCLUDETEXT** mostra texto de outro documento. O pacote guarda uma cópia de cada arquivo, e o campo passa a apontar para essa cópia.
- **Fontes.** Uma fonte é o desenho das letras. O pacote guarda os arquivos de fonte que acompanham o programa e que o documento usa, em uma pasta chamada **fonts**. O Pack and Go nunca empacota uma fonte cuja licença proíba sua distribuição. A mensagem informa cada fonte que ficou de fora. As fontes instaladas no seu computador por você não são empacotadas, por isso a outra pessoa pode precisar delas.

## Como os vínculos mudam

Na cópia, cada vínculo que foi empacotado aponta para o arquivo dentro do pacote, por um nome curto, como **linked/photo.png**. Os vínculos não usam mais o caminho do seu computador, então funcionam onde quer que o pacote seja aberto. Se dois arquivos tiverem o mesmo nome, ao segundo é acrescentado um número.

Um vínculo que o Pack and Go não consegue seguir permanece como estava, e a mensagem o lista. O Pack and Go não empacota:

- Um arquivo que não pode ser encontrado.
- Um endereço da web. Esses permanecem como estão.
- Um arquivo em outro computador da sua rede.
- Um vínculo que sobe e sai da pasta do documento, como **../../notas.png**.
- Um documento, pasta de trabalho ou outro objeto que não esteja na pasta do próprio documento (ou em uma pasta dentro dela), mesmo quando o vínculo informa o caminho completo. Somente imagens, vídeos e sons são empacotados de qualquer lugar.
- Um vínculo que sai da pasta do documento por meio de um atalho para outra pasta (um link simbólico).
- Um vínculo para o próprio documento.
- Um arquivo que não seja imagem, vídeo, som ou documento. Um programa nunca é empacotado.

O Pack and Go não procura vínculos dentro dos arquivos vinculados, por isso um arquivo que aponte de volta para este não pode causar um ciclo.

## Limites

Para que um pacote não fique grande demais, o Pack and Go pergunta antes quando há mais de 200 arquivos vinculados ou mais de 256 MB deles. Escolha **Não** e nada é salvo. O Pack and Go nunca empacota mais de 2.000 arquivos ou mais de 1 GB, qualquer que seja a sua resposta. Os arquivos além desse limite ficam de fora e são listados na mensagem.

Se você usar uma senha no documento, a cópia no pacote terá a mesma senha.
