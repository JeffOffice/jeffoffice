# Mala direta e mesclagem de catálogo
<!-- keywords: mala direta, mesclar, mesclagem, destinatários, lista de endereços, etiquetas, cartas modelo, carta padrão, csv, xlsx, vcard, campo de mesclagem, bloco de endereço, catálogo, lista de produtos, mesclagem de email, visualizar -->

A mala direta preenche uma publicação com dados de uma lista. Você escreve uma carta uma única vez, e o JeffPub faz uma cópia para cada pessoa, com o nome e o endereço dela no lugar certo. A mesclagem de catálogo repete um layout para cada item de uma lista.

As ferramentas ficam na guia **Correspondências**. Para seguir um passo a passo, clique em **Correspondências > Iniciar > Mala Direta > Assistente de mala direta passo a passo** para abrir o painel **Mala Direta**.

## Escolher os destinatários

Clique em **Correspondências > Iniciar > Selecionar Destinatários**:

- **Usar uma lista existente…**: escolha um arquivo CSV, um arquivo de texto separado por tabulações, uma planilha .xlsx (a primeira planilha) ou um arquivo de contatos .vcf. Em um arquivo CSV, de texto ou de planilha, a primeira linha deve conter os nomes das colunas.
- **Selecionar dos contatos…**: escolha um arquivo de contatos (.vcf ou .vcard). A maioria dos catálogos de endereços e celulares consegue salvar os contatos nesse formato; cada contato se torna um destinatário, com colunas de nome, endereço, telefone e email.
- **Digitar uma nova lista…**: preencha a janela **Nova Lista de Endereços**, usando **Nova Entrada** para cada pessoa. Em seguida, o JeffPub oferece a opção de salvar a lista como um arquivo CSV.

Para conferir a lista, clique em **Correspondências > Iniciar > Editar lista de destinatários**. Na janela **Destinatários da Mala Direta**, desmarque uma linha para ignorar essa pessoa, clique em um título de coluna para classificar e digite em **Filtrar destinatários** para restringir a lista. **Localizar Duplicatas** desmarca as repetições, e **Validar Endereços** realça as linhas com endereço ausente ou CEP estranho.

## Adicionar campos de mesclagem

Um campo de mesclagem é um espaço reservado, como «Nome», que se torna um valor real para cada pessoa. Clique em uma caixa de texto (o JeffPub adiciona uma se você não fizer isso) e use **Correspondências > Gravar e Inserir Campos**:

- **Bloco de endereço**: um endereço para correspondência montado a partir de colunas com nomes como Nome, Sobrenome e Linha de Endereço 1.
- **Linha de saudação**: "Prezado(a)" e o nome da pessoa, ou "Prezado(a) amigo(a)," se não houver nome.
- **Inserir Campo de Mesclagem**: escolha qualquer coluna.
- **Campo de Imagem**: escolha uma coluna com nomes de arquivos de imagem. O JeffPub adiciona um quadro vazio e procura os arquivos na mesma pasta da lista. Depois que você salva a publicação e a abre novamente, uma imagem cujo arquivo esteja fora da pasta da publicação não aparece até que você escolha a lista de destinatários de novo (**Correspondências > Selecionar Destinatários > Usar uma lista existente…**). Então, as imagens da pasta da lista e das pastas dentro dela aparecem até você fechar a publicação. O JeffPub não guarda essa escolha para a próxima vez. Outra opção é manter os arquivos de imagem na pasta da publicação, ou em uma pasta dentro dela, para que apareçam assim que a publicação for aberta. Isso impede que uma publicação enviada por outra pessoa leia imagens do seu computador por conta própria. A imagem de cada registro aparece quando você visualiza os resultados, imprime, salva um PDF ou mescla em uma nova publicação ou em um catálogo.

## Visualizar os resultados

Clique em **Correspondências > Visualizar Resultados > Visualizar Resultados** para ver os dados reais no lugar dos campos. Use **Primeiro registro**, **Registro anterior**, **Próximo registro** e **Último registro** para percorrer as pessoas, ou **Localizar destinatário** para ir até uma correspondência.

## Concluir a mesclagem

Clique em **Correspondências > Concluir > Concluir e mesclar**:

- **Mesclar para Impressora…** abre a página **Imprimir**. Marque **Imprimir todos os registros da mala direta** e imprima.
- **Mesclar em nova publicação** cria uma nova publicação com uma cópia das suas páginas para cada pessoa.
- **Mesclar para PDF…** salva um único PDF com as páginas de todas as pessoas.
- **Mesclar para Email…** cria um arquivo de email .eml por pessoa, com a primeira página como imagem. Escolha a coluna de email, digite um assunto e selecione uma pasta. O JeffPub não envia os emails. Abra os arquivos no seu programa de email.
- **Exportar Lista de Destinatários…** salva a lista como um arquivo CSV.

## Mesclagem de catálogo

Clique em **Inserir > Páginas > Páginas de catálogo** para abrir o painel **Mesclagem de catálogo**.

1. Escolha a lista de produtos, como descrito acima.
2. Clique em **Inserir área de catálogo**. Aparece uma caixa identificada como "Área de catálogo". Defina as **Linhas** e **Colunas**, além da posição e do tamanho. Coloque os campos de um item na primeira célula clicando duas vezes neles na lista. Eles se repetem nas outras células. Escolha a versão "(imagem)" de um campo para uma imagem.
3. Visualize com as setas. Cada página mostra tantos itens quanto o número de células da área. Depois, mescle em uma nova publicação, em um PDF ou na impressora.

**Remover Área de Catálogo** remove a área.

## Relacionados

- [Imprimir uma publicação](printing)
- [Adicionar e formatar caixas de texto](text-boxes)
- [A guia Correspondências](tab-mailings)
