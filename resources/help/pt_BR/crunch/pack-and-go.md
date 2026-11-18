# Pack and Go: enviar uma pasta de trabalho com o que ela precisa
<!-- keywords: pack and go, empacotar, pacote, enviar, compartilhar, outro computador, zip, pasta, fontes, vínculos, arquivos vinculados, imagens vinculadas, imagens ausentes, copiar, levar consigo, pendrive, licença, vínculo externo, pasta de trabalho vinculada, células, gráficos -->

Quando você envia uma pasta de trabalho para alguém, as imagens e as outras pastas de trabalho às quais ela está vinculada podem se perder no caminho. O **Pack and Go** salva a pasta de trabalho junto com esses arquivos e com as suas fontes, em um único arquivo ZIP ou em uma única pasta, para que ela tenha a mesma aparência em outro computador.

## Como usar

1. Clique em **Arquivo** e depois em **Exportar**.
2. Clique em **Pack and Go: Salvar para Outro Computador** para criar um único arquivo ZIP, ou em **Pack and Go: Salvar em uma Pasta** para criar uma nova pasta.
3. Escolha onde salvar. Para um arquivo ZIP, digite um nome. Para uma pasta, escolha a pasta na qual a nova será criada.
4. Leia a mensagem que aparece. Ela informa onde está o pacote, quantos arquivos vinculados e arquivos de fonte foram incluídos e tudo o que não pôde ser empacotado.

Um **arquivo ZIP** é um único arquivo que contém outros arquivos, por isso é fácil de enviar. A outra pessoa o abre para extrair os arquivos. Uma pasta é melhor quando você quer copiar tudo para um pendrive.

## O que vai para o pacote

- **A própria pasta de trabalho**, com o mesmo nome que tem agora. É uma cópia. Seu arquivo original não muda.
- **Imagens vinculadas.** Uma imagem da pasta de trabalho pode estar vinculada ao seu arquivo. O pacote contém uma cópia de cada uma. As imagens inseridas diretamente na pasta de trabalho já estão dentro dela.
- **Pastas de trabalho vinculadas.** Uma fórmula pode obter números de outra pasta de trabalho. O pacote contém uma cópia dessa pasta também, e o vínculo passa a apontar para a cópia. As fontes são as usadas em células, gráficos e formas.
- **Fontes.** Uma fonte é o estilo visual das letras. O pacote contém os arquivos de fonte que acompanham o programa e que a pasta de trabalho usa, em uma pasta chamada **fonts**. O Pack and Go nunca empacota uma fonte cuja licença não permita sua distribuição. A mensagem informa cada fonte que ficou de fora. As fontes instaladas no computador por você não são empacotadas, por isso a outra pessoa pode precisar delas.

## Como os vínculos mudam

Na cópia, cada vínculo que foi empacotado aponta para o arquivo dentro do pacote, por um nome curto, como **vinculados/foto.png**. Os vínculos não usam mais o caminho do seu computador, por isso funcionam onde quer que o pacote seja aberto. Se dois arquivos tiverem o mesmo nome, o segundo recebe um número acrescentado.

Um vínculo que o Pack and Go não consegue seguir permanece como estava, e a mensagem o lista. O Pack and Go não empacota:

- Um arquivo que não pode ser encontrado.
- Um endereço da web. Eles permanecem como estão.
- Um arquivo em outro computador da sua rede.
- Um vínculo que sobe e sai da pasta da pasta de trabalho, como **../../notas.png**.
- Um documento, uma pasta de trabalho ou outro objeto que não esteja na própria pasta da pasta de trabalho (ou em uma pasta dentro dela), mesmo quando o vínculo informa o caminho completo. Somente imagens, vídeos e sons são empacotados de qualquer lugar.
- Um vínculo que sai da pasta da pasta de trabalho por meio de um atalho para outra pasta (um link simbólico).
- Um vínculo para a própria pasta de trabalho.
- Um arquivo que não seja imagem, vídeo, som ou documento. Um programa nunca é empacotado.

O Pack and Go não procura outros vínculos dentro dos arquivos vinculados, por isso um arquivo que aponte de volta para este não pode causar um ciclo.

## Limites

Para que o pacote não fique grande demais, o Pack and Go pergunta antes quando há mais de 200 arquivos vinculados ou mais de 256 MB deles. Escolha **Não** e nada será salvo. O Pack and Go nunca empacota mais de 2.000 arquivos ou mais de 1 GB, qualquer que seja a sua resposta. Os arquivos além desse limite são deixados de fora e listados na mensagem.

Se você usar uma senha na pasta de trabalho, a cópia no pacote terá a mesma senha.
